#include <map>
#include <set>
#include <stdlib.h>

#include "fxtimer.h"
#include "fxmeta.h"
#include "lock.h"
#include "limiter_queue.h"
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

class FxTimerHandler: public IFxTimerHandler, TLimiterQueue<double>/*, IFxThread*/
{
private:
	virtual bool CheckEvent(double c)
	{
		if (c <= m_qwSecond)
		{
			return true;
		}
		return false;
	}

public:
	FxTimerHandler()
		: m_qwSecond(0)
		, m_qwDeltaTime(0)
	{
		m_qwSecond = GetTimeOfDay();
		m_strTime[0] = 0;
		tm tmLocal, tmGM;
		time_t t = time(NULL);
		tmLocal = *localtime(&t);
		tmGM = *gmtime(&t);
		tmLocal.tm_isdst = 0;
		tmGM.tm_isdst = 0;
		m_dwTimeZone = (int)(mktime(&tmGM) - mktime(&tmLocal)) / 3600;

		m_dwDayTimeStart = (unsigned int)(t - (t - m_dwTimeZone * 3600) % 86400);

		strftime(m_strTime, 64, "%Y-%m-%d %H:%M:%S", &tmLocal);

		//设置随机数种子
		srand((unsigned int)m_qwSecond);
		//srand(0);
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
		return true;
	}

	virtual void Uninit()
	{
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

	virtual const double GetDeltaTime()
	{
		return m_qwDeltaTime;
	}

	// 添加事件 (多长事件后执行, 事件指针)
	virtual bool AddTimer(double dSecond, CEventBase* pEvent)
	{
		return PushEvent(GetMilliSecond() + dSecond, pEvent) != NULL;
	}

	// 添加事件 (每多少秒执行, 事件指针) 被60整除
	virtual bool AddSecondTimer(unsigned int dwSecond, CEventBase* pEvent)
	{
		return PushEvent(GetSecond() - (GetSecond() % dwSecond) + dwSecond, pEvent) != NULL;
	}

	// 添加事件 (每多少分执行, 事件指针) 被60整除
	virtual bool AddMinuteTimer(unsigned int dwMinute, CEventBase* pEvent)
	{
		return PushEvent(GetSecond() - GetSecond() % (dwMinute * 60) + dwMinute * 60, pEvent) != NULL;
	}

	// 添加事件 (每多少小时执行, 事件指针) 被24整除
	virtual bool AddHourTimer(unsigned int dwHour, CEventBase* pEvent)
	{
		unsigned int dwH1 = (GetSecond() - m_dwDayTimeStart) / 3600;
		return PushEvent(m_dwDayTimeStart + (dwH1 - dwH1 % dwHour + dwHour) * 3600, pEvent) != NULL;
	}

	// 添加事件 (每天几点执行, 事件指针)
	virtual bool AddDayHourTimer(unsigned int dwHour, CEventBase* pEvent)
	{
		unsigned int dwTime = m_dwDayTimeStart + dwHour * 3600;
		if (dwTime <= GetSecond())
		{
			dwTime += 86400;
		}

		return PushEvent(dwTime, pEvent) != NULL;
	}

private:

	void __Refresh()
	{
		static double s_qwSecond = 0;
		static time_t s_dwTime = 0;
		s_qwSecond = GetTimeOfDay();

		m_qwDeltaTime = s_qwSecond - m_qwSecond;
		if (m_qwDeltaTime >= 0.1f)
		{
			LogExe(LogLv_Critical, "FPS : %f", 1 / m_qwDeltaTime);
		}

		Proc((int)(1 / m_qwDeltaTime), s_qwSecond);
		if((unsigned int)s_qwSecond == (unsigned int)m_qwSecond)
		{
			m_qwSecond = s_qwSecond;

			return;
		}
		m_qwSecond = s_qwSecond;

		s_dwTime = (time_t)m_qwSecond;
		if (s_dwTime - m_dwDayTimeStart >= 86400)
		{
			m_dwDayTimeStart += 86400;
		}

		tm* tmLocal = localtime(&s_dwTime); //转化为本地时间
		strftime(m_strTime, 64, "%Y-%m-%d %H:%M:%S", tmLocal);
	}

private:
	double m_qwSecond;
	char m_strTime[64];
	double m_qwDeltaTime;

	int m_dwTimeZone;
	unsigned int m_dwDayTimeStart;
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
