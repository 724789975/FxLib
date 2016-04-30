#ifndef __THREAD_H__
#define __THREAD_H__

#include <stdarg.h>

class IFxThread
{
public:
	virtual ~IFxThread()
	{
	}

	virtual void ThrdFunc() = 0;

	virtual void Stop() = 0;
};

class IFxThreadHandler
{
public:
	virtual ~IFxThreadHandler()
	{
	}

	virtual void Stop(void) = 0;

	virtual bool Kill(unsigned int dwExitCode) = 0;

	virtual bool WaitFor(unsigned int dwWaitTime = 0xffffffff) = 0;

	virtual unsigned int GetThreadId(void) = 0;

	virtual IFxThread* GetThread(void) = 0;

	virtual void Release(void) = 0;

	bool IsStop(void)
	{
		return m_bIsStop;
	}

#ifdef WIN32
	void* GetHandle(void)
	{
		return m_hHandle;
	}
protected:
	void* m_hHandle;		// ????????? ?????? NULL ???? INVALID_HANDLE_VALUE??
#endif // WIN32

protected:
	bool m_bIsStop;
};


IFxThreadHandler* FxCreateThreadHandler(IFxThread* poThread, bool bNeedWaitfor);

#endif // !__THREAD_H__
