#include <map>
#include <set>
#include <stdlib.h>

#include "fxtimer.h"
#include "fxmeta.h"
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
		tm tmLocal, tmGM;
		time_t t = time(NULL);
		tmLocal = *localtime(&t);
		tmGM = *gmtime(&t);
		tmLocal.tm_isdst = 0;
		tmGM.tm_isdst = 0;
		m_dwTimeZone = (int)(mktime(&tmGM) - mktime(&tmLocal)) / 3600;

		m_dwDayTimeStart = (unsigned int)(t - (t - m_dwTimeZone * 3600) % 86400);

		//设置随机数种子
		srand(m_qwSecond);
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

	virtual bool AddDelayTimer(double dSecond, IFxTimer* pFxTimer)
	{
		double dInvokeTime = m_qwSecond + dSecond;
		if (m_mapDelayTimers[dInvokeTime].find(pFxTimer)
				!= m_mapDelayTimers[dInvokeTime].end())
		{
			return false;
		}
		m_mapDelayTimers[dInvokeTime].insert(pFxTimer);
		return true;
	}

	virtual bool DelDelayTimer(IFxTimer* pFxTimer)
	{
		bool bDel = false;
		for (std::map<double, std::set<IFxTimer*> >::iterator it =
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

	virtual unsigned int GetSecond()
	{
		return (unsigned int)m_qwSecond;
	}

	virtual double GetMilliSecond()
	{
		return m_qwSecond;
	}

	virtual const char* GetTimeStr()
	{
		return m_strTime;
	}

	virtual const unsigned int GetTimeSeq()
	{
		return (unsigned int)((m_qwSecond - (unsigned int)m_qwSecond) * 1000);
	}

	virtual const int GetTimeZone()
	{
		return m_dwTimeZone;
	}

	virtual const int GetDayTimeStart()
	{
		return m_dwDayTimeStart;
	}

private:

	void ProcDelayTimer()
	{
		for (std::map<double, std::set<IFxTimer*> >::iterator it =
			m_mapDelayTimers.begin(); it != m_mapDelayTimers.end();)
		{
			if (it->first <= m_qwSecond)
			{
				for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
					itTimer != it->second.end(); ++itTimer)
				{
					(*itTimer)->OnTimer(m_qwSecond);
				}
				m_mapDelayTimers.erase(it++);
			}
			else
			{
				break;
			}
		}
	}

	void __Refresh()
	{
		static double s_qwSecond = 0;
		static time_t s_dwTime = 0;
		s_qwSecond = GetTimeOfDay();

		if((unsigned int)s_qwSecond == (unsigned int)m_qwSecond)
		{
			m_qwSecond = s_qwSecond;

			ProcDelayTimer();
			return;
		}
		m_qwSecond = s_qwSecond;

		s_dwTime = (time_t)m_qwSecond;
		if (s_dwTime - m_dwDayTimeStart >= 86400)
		{
			m_dwDayTimeStart += 86400;
		}

		tm* tmLocal = localtime(&s_dwTime); //转为本地时间  
		strftime(m_strTime, 64, "%Y-%m-%d %H:%M:%S", tmLocal);
		//sprintf(m_strTime, "%s",
		//		CLuaEngine::Instance()->CallStringFunction<unsigned int>(
		//				"GetTimeStr", m_qwSecond));

		ProcDelayTimer();

		for (std::map<unsigned int, std::set<IFxTimer*> >::iterator it =
				m_mapTimers.begin(); it != m_mapTimers.end(); ++it)
		{
			if ((unsigned int)m_qwSecond % it->first == 0)
			{
				for (std::set<IFxTimer*>::iterator itTimer = it->second.begin();
						itTimer != it->second.end(); ++itTimer)
				{
					(*itTimer)->OnTimer(m_qwSecond);
				}
			}
		}

	}

private:
//	bool m_bStop;
	double m_qwSecond;
	char m_strTime[64];

	int m_dwTimeZone;
	unsigned int m_dwDayTimeStart;

	FxCriticalLock m_oLock;

	std::map<unsigned int, std::set<IFxTimer*> > m_mapTimers;
	std::map<double, std::set<IFxTimer*> > m_mapDelayTimers;
};

IFxTimerHandler* GetTimeHandler()
{
	static FxTimerHandler oTimerHandler;
	return &oTimerHandler;
}

unsigned int IFxTimerHandler::GetTimeStampFromStr(const char* szTimeStr)
{
	tm _tm;

#ifdef WIN32
	sscanf_s(szTimeStr, "%4d-%2d-%2d %2d:%2d:%2d",
#else
	sscanf(szTimeStr, "%4d-%2d-%2d %2d:%2d:%2d",
#endif //!WIN32
		&_tm.tm_year, &_tm.tm_mon, &_tm.tm_mday, &_tm.tm_hour, &_tm.tm_min, &_tm.tm_sec);
	_tm.tm_year -= 1900;
	_tm.tm_mon -= 1;
	_tm.tm_isdst = 0;

	return (unsigned int)mktime(&_tm);
}
