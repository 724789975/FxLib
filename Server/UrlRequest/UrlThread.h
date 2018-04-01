#ifndef __UrlThread_H__
#define __UrlThread_H__


#include <list>
#include <time.h>
#include <fxmeta.h>
#include "lock.h"
#include "fxcurl.h"

class CUrlThread : public IFxThread
{
public:
	CUrlThread();
	virtual ~CUrlThread();

	virtual void			ThrdFunc();
	virtual void			Stop();

	bool		    	    Start();

	bool		    	    AddRequest(IUrlRequest* poRequest);

private:
	void					ClearRequest();
	bool					DoRequest();
private:
	FxCriticalLock			m_oLock;
	IFxThreadHandler*		m_poThrdHandler;
	std::list<IUrlRequest*>	m_oRequests;
	bool					m_bTerminate;

	FILE*					m_pFile;
	char					m_szLogPath[64];
};

#endif // !__UrlThread_H__
