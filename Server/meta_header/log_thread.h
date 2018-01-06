#ifndef __LOG_H__
#define __LOG_H__

#include "singleton.h"
#include "thread.h"
#include <stdio.h>

#define LOGLENGTH 512*1024

class IFxLock;

enum LogType
{
	LT_None = 0,
	LT_Screen = 1,
	LT_File = 1 << 1,
};
class LogThread : public TSingleton<LogThread>, public IFxThread
{
public:
	LogThread();
	virtual ~LogThread(){}

	virtual void ThrdFunc();
	virtual void Stop();
	bool Start();
	bool Init();

	void ReadLog(unsigned int dwLogType, char* strLog);
	FILE* GetLogFile();
protected:
private:
	bool					m_bStop;
	
	unsigned int			m_dwInIndex;
	unsigned int			m_dwOutIndex;

	IFxLock*				m_pLock;
	IFxThreadHandler*		m_poThrdHandler;

	char					m_strScreenLog[2][LOGLENGTH];
	char					m_strFileLog[2][LOGLENGTH];

	bool					m_bPrint;
	unsigned int			m_dwCurrentIndex;
};

#endif // !__LOG_H__
