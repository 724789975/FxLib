#include <map>
#include <set>

#include "fxtimer.h"
#include "fxmeta.h"
#include "lua_engine.h"
#include "lock.h"
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif // WIN32

double GetTimeOfDay()
{
	static double s_qwTime = 0;
#ifdef WIN32
	s_qwTime = (double)time(NULL);
	SYSTEMTIME st;
	GetSystemTime(&st);
	s_qwTime = s_qwTime + st.wMilliseconds / 1000.0f;
#else
	static struct timeval tv;
	gettimeofday(&tv, NULL);
	s_qwTime = tv.tv_sec / 1.0 + tv.tv_usec / 1000000.0;
#endif
	return s_qwTime;
}

class FxTimerHandler: public IFxTimerHandler/*, IFxThread*/
{
public:
	FxTimerHandler() :
		m_qwSecond(0)
	{
		m_strTime[0] = 0;
//		m_bStop = false;
	}

	virtual ~FxTimerHandler()
	{
	}

	virtual void Run()
	{
		__Refresh();
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
		unsigned int dwInvokeTime = (unsigned int)m_qwSecond + dwSecond;
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
		return (int)m_qwSecond;
	}

	virtual const char* GetTimeStr()
	{
		return m_strTime;
	}

	virtual const unsigned int GetTimeSeq()
	{
		return (unsigned int)((m_qwSecond - (unsigned int)m_qwSecond) * 1000);
	}

private:
	void __Refresh()
	{
		static double s_qwSecond = 0;
		static time_t s_dwTime = 0;
		s_qwSecond = GetTimeOfDay();

		if((unsigned int)s_qwSecond == (unsigned int)m_qwSecond)
		{
			m_qwSecond = s_qwSecond;
			return;
		}

		m_qwSecond = s_qwSecond;

		s_dwTime = (time_t)m_qwSecond;
		tm* tmLocal = localtime(&s_dwTime); //转为本地时间  
		strftime(m_strTime, 64, "%Y-%m-%d %H:%M:%S", tmLocal);
		//sprintf(m_strTime, "%s",
		//		CLuaEngine::Instance()->CallStringFunction<unsigned int>(
		//				"GetTimeStr", m_qwSecond));


		for (std::map<unsigned int, std::set<IFxTimer*> >::iterator it =
				m_mapTimers.begin(); it != m_mapTimers.end(); ++it)
		{
			if ((unsigned int)m_qwSecond % it->first == 0)
			{
				for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
						itTimer != it->second.end(); ++itTimer)
				{
					(*itTimer)->OnTimer((unsigned int)m_qwSecond);
				}
			}
		}

		for (std::map<unsigned int, std::set<IFxTimer*> >::iterator it =
				m_mapDelayTimers.begin(); it != m_mapDelayTimers.end();)
		{
			if (it->first <= m_qwSecond)
			{
				for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
						itTimer != it->second.end(); ++itTimer)
				{
					(*itTimer)->OnTimer((unsigned int)m_qwSecond);
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
	double m_qwSecond;
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

