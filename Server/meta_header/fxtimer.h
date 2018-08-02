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

	// ����¼� (�೤�¼���ִ��, �¼�ָ��)
	virtual bool AddTimer(double dSecond, CEventBase* pEvent) = 0;
	// ����¼� (ÿ������ִ��, �¼�ָ��) ��60����
	virtual bool AddSecontTimer(unsigned int dwSecond, CEventBase* pEvent) = 0;
	// ����¼� (ÿ���ٷ�ִ��, �¼�ָ��) ��60����
	virtual bool AddMinuteTimer(unsigned int dwMinute, CEventBase* pEvent) = 0;
	// ����¼� (ÿ����Сʱִ��, �¼�ָ��) ��24����
	virtual bool AddHourTimer(unsigned int dwHour, CEventBase* pEvent) = 0;
	// ����¼� (ÿ�켸��ִ��, �¼�ָ��)
	virtual bool AddDayHourTimer(unsigned int dwHour, CEventBase* pEvent) = 0;

	//��ȡʱ��
	virtual const int GetTimeZone() = 0;

	//��ȡ����0���ʱ���(����ʱ��)
	virtual const int GetDayTimeStart() = 0;

	unsigned int GetTimeStampFromStr(const char* szTimeStr);
};

IFxTimerHandler* GetTimeHandler();

#endif /* FXTIMER_H_ */
