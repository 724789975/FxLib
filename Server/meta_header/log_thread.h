#ifndef __LOG_H__
#define __LOG_H__

#include "singleton.h"
#include "thread.h"
#include <stdio.h>
#include <sstream>

#define LOGLENGTH 512*1024

class IFxLock;

enum LogType
{
	LT_None = 0,
	LT_Screen = 1,
	LT_File = 1 << 1,
};

class LogItem
{
public:
	LogItem();
	void									Reset();
	std::stringstream&						GetStream();
private:
	char									m_szScreenLog[LOGLENGTH];
	unsigned int							m_dwLogLength;

	std::stringstream						m_streamLogStream;
};

class LogThread;

class LogImp
{
public:
	LogImp(LogThread& refLog);
	~LogImp();
protected:
private:
	LogThread& m_refLog;
};

class LogThread : public IFxThread
{
	friend class LogImp;
public:
	LogThread(bool bPrintScene);
	virtual ~LogThread(){}

	virtual void							ThrdFunc();
	virtual void							Stop();
	bool									Start();
	bool									Init();

	std::stringstream						GetStream();

	void									ReadLog(unsigned int dwLogType, char* strLog);
	FILE*									GetLogFile();
protected:
private:
	bool									m_bStop;
	
	unsigned int							m_dwInIndex;
	unsigned int							m_dwOutIndex;

	IFxLock*								m_pLock;
	IFxThreadHandler*						m_poThrdHandler;

	char									m_strScreenLog[2][LOGLENGTH];
	char									m_strFileLog[2][LOGLENGTH];

	bool									m_bPrint;

	LogItem									m_oLogItem[2];

	unsigned int							m_dwCurrentIndex;				//��ǰΪ��д +1 Ϊд��

	bool									m_bPrintScene;					//�Ƿ�����Ļ�ϴ�ӡ(���߳�����Ļ�ϴ�ӡ ����ֻ��ӡlog)
};

#endif // !__LOG_H__
