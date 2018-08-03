#include "GameServerBase.h"

#ifdef WIN32
#include <Windows.h>
#include <Psapi.h>
#endif



CGameServerBase::CGameServerBase()
{
}


CGameServerBase::~CGameServerBase()
{
}

void CGameServerBase::GetLoadPressure(unsigned short& wCPU, unsigned short& wMEMP, unsigned short& wMEM)
{
	wCPU = 0;
	wMEMP = 0;
	wMEM = 0;
#ifndef _WIN32
	char cmd[128];
	int pid = getpid();
	_snprintf(cmd, 128, "ps -o %%cpu,rss,%%mem,pid,tid -mp %d", pid);
	FILE* file;
	file = popen(cmd, "r");
	if (file == NULL)
	{
		LogExe(LogLv_Critical, "file == NULL");
		return;
	}

	char __line[256];
	float __cpuPercent = 0;
	int __mem = 0;
	float __memPercent = 0;
	int __pid = 0;
	int __tid = 0;
	if (fgets(__line, 256, file) != NULL)
	{
		if (fgets(__line, 256, file) != NULL)
		{
			sscanf(__line, "%f %d %f %d %d", &__cpuPercent, &__mem, &__memPercent, &__pid, &__tid);
			wCPU = (unsigned short)(__cpuPercent * 100);
			wMEMP = (unsigned short)(__memPercent * 100);
			wMEM = (unsigned short)(__mem / 1024);
		}
	}
	pclose(file);
#else
	// NtQuerySystemInformation

	HANDLE m_hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;

	GetProcessMemoryInfo(m_hProcess, &pmc, sizeof(pmc));

	wCPU = 0;
	wMEMP = (unsigned short)(pmc.WorkingSetSize / 1024);

#endif
}
