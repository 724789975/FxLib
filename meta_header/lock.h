#ifndef __Lock_H__
#define __Lock_H__

#include "fxmeta.h"

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class FxCriticalLock: public IFxLock
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

#endif /* __Lock_H__ */
