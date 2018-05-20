#ifndef __Timers_H__
#define __Timers_H__

#include "fxtimer.h"

class GameEnd : public IFxTimer
{
	virtual bool OnTimer(double fSecond);
};

class GameStart : public IFxTimer
{
	virtual bool OnTimer(double fSecond);
};


#endif // __Timers_H__
