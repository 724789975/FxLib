#ifndef FXTIMER_H_
#define FXTIMER_H_

#include "limiter_queue.h"

class IFxTimer
{
public:
//	IFxTimer(unsigned int dwDelayTime = 0) :
//			m_dwDelay(dwDelayTime)
	IFxTimer()
	{
	}
	virtual ~IFxTimer()
	{
	}
	virtual bool OnTimer(double fSecond) = 0;

private:
//	unsigned int m_dwDelayTime;
};

class IFxTimerHandler
{
public:
	virtual ~IFxTimerHandler()
	{
	}

	virtual bool Init() = 0;
	virtual void Uninit() = 0;
	virtual void Run() = 0;
	virtual bool AddDelayTimer(double dSecond, IFxTimer* pFxTimer) = 0;
	virtual bool DelDelayTimer(IFxTimer* pFxTimer) = 0;
	virtual bool AddEveryFewMinuteTimer(unsigned int dwSecond, IFxTimer* pFxTimer) = 0;
	virtual bool DelEveryFewMinuteTimer(IFxTimer* pFxTimer) = 0;
	virtual unsigned int GetSecond() = 0;
	virtual double GetMilliSecond() = 0;
	virtual const char* GetTimeStr() = 0;
	virtual const unsigned int GetTimeSeq() = 0;
	virtual const double GetDeltaTime() = 0;

	// 添加事件 (多长事件后执行, 事件指针)
	virtual bool AddTimer(double dSecond, CEventBase* pEvent) = 0;
	// 添加事件 (每多少秒执行, 事件指针) 被60整除
	virtual bool AddSecontTimer(unsigned int dwSecond, CEventBase* pEvent) = 0;
	// 添加事件 (每多少分执行, 事件指针) 被60整除
	virtual bool AddMinuteTimer(unsigned int dwMinute, CEventBase* pEvent) = 0;
	// 添加事件 (每多少小时执行, 事件指针) 被24整除
	virtual bool AddHourTimer(unsigned int dwHour, CEventBase* pEvent) = 0;
	// 添加事件 (每天几点执行, 事件指针)
	virtual bool AddDayHourTimer(unsigned int dwHour, CEventBase* pEvent) = 0;

	//获取时区
	virtual const int GetTimeZone() = 0;

	//获取今天0点的时间戳(当地时区)
	virtual const int GetDayTimeStart() = 0;

	unsigned int GetTimeStampFromStr(const char* szTimeStr);
};

IFxTimerHandler* GetTimeHandler();

#endif /* FXTIMER_H_ */
