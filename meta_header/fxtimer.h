#ifndef FXTIMER_H_
#define FXTIMER_H_

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
	virtual bool OnTimer(unsigned int dwSecond) = 0;

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
	virtual bool AddDelayTimer(unsigned int dwSecond, IFxTimer* pFxTimer) = 0;
	virtual bool DelDelayTimer(IFxTimer* pFxTimer) = 0;
	// ???????//
	virtual bool AddEveryFewMinuteTimer(unsigned int dwSecond, IFxTimer* pFxTimer) = 0;
	virtual bool DelEveryFewMinuteTimer(IFxTimer* pFxTimer) = 0;
	virtual int GetSecond() = 0;
	virtual char* GetTimeStr() = 0;
	virtual unsigned int GetTimeSeq() = 0;
};

IFxTimerHandler* GetTimeHandler();

#endif /* FXTIMER_H_ */
