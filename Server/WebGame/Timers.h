#ifndef __Timers_H__
#define __Timers_H__

#include "fxtimer.h"

class GameEnd : public CEventCaller<GameEnd, 1>
{
public:
	bool OnTimer(double fSecond);
};

class GameStart : public CEventCaller<GameEnd, 1>
{
public:
	bool OnTimer(double fSecond);
};


#endif // __Timers_H__
