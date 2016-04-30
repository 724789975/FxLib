#ifndef __LOG_H__
#define __LOG_H__

#include "singleton.h"
#include "thread.h"
#include <stdio.h>

#define LOGLENGTH 2048
#define LOGITEMNUM 16

class IFxLock;

enum LogType
{
	LT_None = 0,
	LT_Screen = 1,
	LT_File = 1 << 1,
};
class LogThread : public TSingletion<LogThread>, public IFxThread
{
public:
	class LogItem
	{
		enum LogState
		{
			LS_None,
			LS_Writing,
			LS_WriteEnd,
		};
		friend class LogThread;
	public:
		LogItem();
		~LogItem();

		void Reset();

	private:
		LogState m_eState;
		unsigned int m_dwLogType;
		char m_strLog[LOGLENGTH];
	};

public:
	LogThread();
	virtual ~LogThread(){}

	virtual void ThrdFunc();
	virtual void Stop();
	bool Start();
	bool Init();

	void BeginLog(unsigned int dwLogType, char* & strLog, unsigned int& dwIndex);
	void EndLog(unsigned int dwIndex);
	FILE* GetLogFile();
protected:
private:
	
	unsigned int			m_dwInIndex;
	unsigned int			m_dwOutIndex;

	IFxLock*				m_pLock;
	IFxThreadHandler*		m_poThrdHandler;

	LogItem					m_oLogItems[LOGITEMNUM];
};

#endif // !__LOG_H__
