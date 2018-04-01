#ifndef __CurlModule_H__
#define __CurlModule_H__

#include "fxcurl.h"

#include <map>
#include <fxmeta.h>
#include "singleton.h"
#include "dynamicpoolex.h"
#include "lock.h"
#include "UrlThread.h"

class FxCurlModule : public TSingleton<FxCurlModule>, public IUrlRequestModule
{
public:
	FxCurlModule();
	virtual ~FxCurlModule();

	virtual bool        Init();
	virtual void        Close();
	virtual bool        AddRequest(IUrlRequest *poRequest);
	virtual bool        Run(UINT32 dwCount = -1);

	void AddResult(IUrlRequest* poRequest);

private:
	bool				CallBackResult();
	void				ClearResult();
private:
	FxCriticalLock		    m_oLock;
	std::list<IUrlRequest*>       m_oResultList;
	CUrlThread				m_oCUrlThread;
};

#endif	!//__CurlModule_H__
