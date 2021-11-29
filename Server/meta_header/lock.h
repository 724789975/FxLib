#ifndef __Lock_H__
#define __Lock_H__

#include "fxmeta.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include<sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>
#endif

class FxCriticalLock : public IFxLock
{
public:
	// Can't be copied
//		void operator =(FxCriticalLock &locker)
//		{
//		}
	public:
	#ifdef _WIN32
		FxCriticalLock()
		{
			InitializeCriticalSection(&m_lock);
		}
		~FxCriticalLock()
		{
			DeleteCriticalSection(&m_lock);
		}
		void Lock()
		{
			EnterCriticalSection(&m_lock);
		}
		void UnLock()
		{
			LeaveCriticalSection(&m_lock);
		}
	#else
		FxCriticalLock()
		{
			pthread_mutex_init(&m_lock, NULL);
		}
		~FxCriticalLock()
		{
			pthread_mutex_destroy(&m_lock);
		}
		void Lock()
		{
			pthread_mutex_lock(&m_lock);
		}
		void UnLock()
		{
			pthread_mutex_unlock(&m_lock);
		}
	#endif

		virtual void Release()
		{
			if (NULL != this)
				delete this;
		}

private:
#ifdef _WIN32
	CRITICAL_SECTION m_lock;
#else
	pthread_mutex_t m_lock;
#endif
};

//进程锁问题 如果进程没有解锁就推出 会出现死锁问题
class ProcessLock : public IFxLock
{
public:
#ifdef _WIN32
	ProcessLock(const char* szName)
	{
		m_hMutex = CreateMutex(NULL, false, szName);
	}
	~ProcessLock()
	{
		CloseHandle(m_hMutex);
	}
	void Lock()
	{
		DWORD d = WaitForSingleObject(m_hMutex, INFINITE);
		if (WAIT_OBJECT_0 == d)
		{
		}
		if (WAIT_ABANDONED == d)
		{
		}
		if (WAIT_FAILED == d)
		{
			assert(0);
		}
	}
	void UnLock()
	{
		ReleaseMutex(m_hMutex);
	}
#else
#if 0
	struct mt
	{
		pthread_mutex_t mutex;
		pthread_mutexattr_t mutexattr;
	};

	ProcessLock(const char* szName)
	{
		int fd = open(szName, O_CREAT | O_RDWR, 0777);
		assert(fd != -1);
		ftruncate(fd, sizeof(*mm));
		mm = (mt*)mmap(NULL, sizeof(*mm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		memset(mm, 0x00, sizeof(*mm));
		pthread_mutexattr_init(&mm->mutexattr);
		pthread_mutexattr_setpshared(&mm->mutexattr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&mm->mutex, &mm->mutexattr);
	}
	~ProcessLock()
	{
		pthread_mutexattr_destroy(&mm->mutexattr);
		pthread_mutex_destroy(&mm->mutex);
	}
	void Lock()
	{
		pthread_mutex_lock(&mm->mutex);
	}
	void UnLock()
	{
		pthread_mutex_unlock(&mm->mutex);
	}
#else
	union semun
	{
		int val;
		struct semid_ds *buf;
		unsigned short *arr;
	};

	ProcessLock(const char* szName)
	{
		int fd = open(szName, O_CREAT | O_RDWR, 0600);
		assert(fd != -1);
		close(fd);
		key_t kt = ftok(szName, 'a');
		assert(kt != -1);
		m_dwSemId = semget((key_t)kt, 1, IPC_CREAT | IPC_EXCL | 0600);
		if (m_dwSemId == -1)
		{
			m_dwSemId = semget(kt, 1, IPC_CREAT | 0600);
			assert(m_dwSemId != -1);
		}
		else
		{
			union semun sem_union;
			sem_union.val = 1;
			if (semctl(m_dwSemId, 0, SETVAL, sem_union) == -1)
			{
				assert(0);
			}
		}

	}
	virtual ~ProcessLock()
	{
		if (semctl(m_dwSemId, 0, IPC_RMID) == -1)
		{
			assert(0);
		}

	}
	void Lock()
	{
		struct sembuf sem_b;
		sem_b.sem_num = 0;
		sem_b.sem_op = -1;
		sem_b.sem_flg = SEM_UNDO;
		if (semop(m_dwSemId, &sem_b, 1) == -1)
		{
			assert(0);
		}

	}
	void UnLock()
	{
		struct sembuf sem_b;
		sem_b.sem_num = 0;
		sem_b.sem_op = 1;
		sem_b.sem_flg = SEM_UNDO;
		if (semop(m_dwSemId, &sem_b, 1) == -1)
		{
			assert(0);
		}
	}
#endif
#endif

	virtual void Release()
	{
		if (NULL != this)
			delete this;
	}

private:
#ifdef _WIN32
	HANDLE m_hMutex;
#else
#if 0
	struct mt* mm;
#else
	int m_dwSemId;
#endif
#endif // _WIN32

};


#endif /* __Lock_H__ */
