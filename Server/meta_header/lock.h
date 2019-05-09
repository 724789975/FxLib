#ifndef __Lock_H__
#define __Lock_H__

#include "fxmeta.h"

#ifdef WIN32
#include <WinSock2.h>
#else
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#endif

class FxCriticalLock : public IFxLock
{
public:
	// Can't be copied
//		void operator =(FxCriticalLock &locker)
//		{
//		}
	public:
	#ifdef WIN32
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
#ifdef WIN32
	CRITICAL_SECTION m_lock;
#else
	pthread_mutex_t m_lock;
#endif
};

class ProcessLock : public IFxLock
{
public:
#ifdef WIN32
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

#endif

	virtual void Release()
	{
		if (NULL != this)
			delete this;
	}

private:
#ifdef WIN32
	HANDLE m_hMutex;
#else
	struct mt* mm;
#endif // WIN32

};


#endif /* __Lock_H__ */
