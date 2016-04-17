#include <map>
#include <set>

#include "fxtimer.h"
#include "fxmeta.h"
#include "lua_engine.h"
#include "lock.h"

class FxTimerHandler: public IFxTimerHandler/*, IFxThread*/
{
public:
	FxTimerHandler() :
		m_dwSecond(0)
	{
		m_strTime[0] = 0;
//		m_bStop = false;
	}

	virtual ~FxTimerHandler()
	{
	}

	virtual void Run()
	{
		{
			__Refresh();
		}
	}

	virtual bool Init()
	{
		m_mapTimers.clear();
		return true;
	}

	virtual void Uninit()
	{
	}

	virtual bool AddDelayTimer(unsigned int dwSecond, IFxTimer* pFxTimer)
	{
		unsigned int dwInvokeTime = m_dwSecond + dwSecond;
		if (m_mapDelayTimers[dwInvokeTime].find(pFxTimer)
				!= m_mapDelayTimers[dwInvokeTime].end())
		{
			return false;
		}
		m_mapDelayTimers[dwInvokeTime].insert(pFxTimer);
		return true;
	}

	virtual bool DelDelayTimer(IFxTimer* pFxTimer)
	{
		bool bDel = false;
		for (std::map<unsigned int, std::set<IFxTimer*> >::iterator it =
				m_mapDelayTimers.begin(); it != m_mapDelayTimers.end(); ++it)
		{
			for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
					itTimer != it->second.end();)
			{
				if (*itTimer == pFxTimer)
				{
					it->second.erase(itTimer++);
					bDel = true;
				}
				else
				{
					++itTimer;
				}
			}
		}
		return bDel;
	}

	virtual bool AddEveryFewMinuteTimer(unsigned int dwSecond,
			IFxTimer* pFxTimer)
	{
		if (m_mapTimers[dwSecond].find(pFxTimer) != m_mapTimers[dwSecond].end())
		{
			return false;
		}
		m_mapTimers[dwSecond].insert(pFxTimer);
		return true;
	}

	virtual bool DelEveryFewMinuteTimer(IFxTimer* pFxTimer)
	{
		bool bDel = false;
		for (std::map<unsigned int, std::set<IFxTimer*> >::iterator it =
				m_mapTimers.begin(); it != m_mapTimers.end(); ++it)
		{
			for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
					itTimer != it->second.end();)
			{
				if (*itTimer == pFxTimer)
				{
					it->second.erase(itTimer++);
					bDel = true;
				}
				else
				{
					++itTimer;
				}
			}
		}
		return bDel;
	}

	virtual int GetSecond()
	{
		return m_dwSecond;
	}

	virtual char* GetTimeStr()
	{
		return m_strTime;
	}

private:
	void __Refresh()
	{
		static unsigned int sdwSecond = 0;
		sdwSecond = (unsigned int) (CLuaEngine::Instance()->CallNumberFunction(
				"GetTimeSecond"));

		if(sdwSecond == m_dwSecond)
		{
			// ͬһ���� ��Щ����ִֻ��һ��//
			return;
		}

		m_dwSecond = sdwSecond;
		sprintf(m_strTime, "%s",
				CLuaEngine::Instance()->CallStringFunction<unsigned int>(
						"GetTimeStr", m_dwSecond));

		for (std::map<unsigned int, std::set<IFxTimer*> >::iterator it =
				m_mapTimers.begin(); it != m_mapTimers.end(); ++it)
		{
			if (m_dwSecond % it->first == 0)
			{
				for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
						itTimer != it->second.end(); ++itTimer)
				{
					(*itTimer)->OnTimer(m_dwSecond);
				}
			}
		}

		for (std::map<unsigned int, std::set<IFxTimer*> >::iterator it =
				m_mapDelayTimers.begin(); it != m_mapDelayTimers.end();)
		{
			if (it->first <= m_dwSecond)
			{
				for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
						itTimer != it->second.end(); ++itTimer)
				{
					(*itTimer)->OnTimer(m_dwSecond);
				}
				m_mapDelayTimers.erase(it++);
			}
			else
			{
				break;
			}
		}
	}

private:
//	bool m_bStop;
	unsigned int m_dwSecond;
	char m_strTime[64];
	FxCriticalLock m_oLock;

	std::map<unsigned int, std::set<IFxTimer*> > m_mapTimers;
	std::map<unsigned int, std::set<IFxTimer*> > m_mapDelayTimers;
};

IFxTimerHandler* GetTimeHandler()
{
	static FxTimerHandler oTimerHandler;
	return &oTimerHandler;
}

