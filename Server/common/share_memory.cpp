#include "share_memory.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h> 
#endif

#include <assert.h>

CShareMem::CShareMem(std::string szName, unsigned long long qwSize)
	: m_szShmName(szName)
	, m_qwSize(qwSize)
#ifdef _WIN32
	, m_hShmId(INVALID_HANDLE_VALUE)
#else
	, m_hShmId(0XFFFFFFFF)
#endif // _WIN32
	, m_pData(NULL)
{
}

CShareMem::~CShareMem()
{
}

bool CShareMem::Init(bool& bCreated, void* pAddr /*= NULL*/)
{
	bCreated = false;
	if (!Open(pAddr))
	{
		bCreated = true;
		return Create(pAddr);
	}
	return true;
}

void CShareMem::DeleteShareMem()
{
#ifdef _WIN32
	// 关闭视图
	if (m_pData != NULL)
	{
		UnmapViewOfFile(m_pData);
	}

	// 关闭内核对象
	if (m_hShmId != NULL)
	{
		CloseHandle(m_hShmId);
	}
#else
	assert(shmdt((void*)m_pData) == 0);
	struct shmid_ds shmbuffer = { 0 };
	assert(shmctl(m_hShmId, IPC_RMID, &shmbuffer) == 0);
#endif

	m_pData = NULL;
}

bool CShareMem::IsExist()
{
#ifdef _WIN32
	HANDLE h_File = CreateFile((m_szShmName + ".shm").c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, NULL, NULL);
	if (h_File != INVALID_HANDLE_VALUE)
	{
		CloseHandle(h_File);
		return true;
	}

#else
	//int fHandle = open(m_szShmName.c_str(), O_RDWR, 0777);
	//if (fHandle != -1)
	//{
	//	return true;
	//}
	FILE* fp = fopen((m_szShmName + ".shm").c_str(), "r");
	if (fp)
	{
		fclose(fp);
		return true;
	}
#endif // _WIN32
	return false;
}

bool CShareMem::Open(void* pAddr)
{
	assert(m_pData == NULL);
#ifdef _WIN32
	HANDLE hFile = CreateFile((m_szShmName + ".shm").c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	//ResizeFile(hFile);

	m_hShmId = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
		(DWORD)m_qwSize, m_szShmName.c_str());
	if (m_hShmId == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	//m_hShmId = OpenFileMapping(FILE_MAP_READ | FILE_SHARE_WRITE, FALSE, m_szShmName.c_str());
	//if (m_hShmId == INVALID_HANDLE_VALUE)
	//{
	//	return false;
	//}

	m_pData = MapViewOfFileEx(m_hShmId, FILE_MAP_READ | FILE_SHARE_WRITE, 0, 0, 0, pAddr);
	if (m_pData == NULL)
	{
		CloseHandle(hFile);
		return false;
	}
#else
	int fHandle = open((m_szShmName + ".shm").c_str(), O_RDWR, 0777);
	if (fHandle == -1)
	{
		return false;
	}
	key_t keyShmKey = ftok((m_szShmName + ".shm").c_str(), 'a');
	if (keyShmKey == -1)
	{
		return false;
	}

	m_hShmId = shmget(keyShmKey, 0, SHM_R | SHM_W);
	if (m_hShmId == -1)
	{
		return false;
	}

	m_pData = shmat(m_hShmId, pAddr, 0);
	if (m_pData == (void*)-1)
	{
		return false;
	}

	assert(m_pData == mmap(m_pData, m_qwSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fHandle, 0));
#endif // _WIN32
	return true;
}

bool CShareMem::Create(void* pAddr)
{
	assert(m_pData == NULL);
#ifdef _WIN32
	HANDLE hFile = CreateFile((m_szShmName + ".shm").c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, CREATE_NEW, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	//ResizeFile(hFile);

	m_hShmId = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
		(DWORD)m_qwSize, m_szShmName.c_str());
	if (m_hShmId == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	m_pData = MapViewOfFileEx(m_hShmId, FILE_MAP_READ | FILE_SHARE_WRITE, 0, 0, 0, pAddr);
	if (m_pData == NULL)
	{
		CloseHandle(hFile);
		return false;
	}
#else
	int fHandle = open((m_szShmName + ".shm").c_str(), O_RDWR | O_CREAT, 0777);
	if (fHandle == -1)
	{
		return false;
	}
	if (0 != ftruncate(fHandle, m_qwSize))
	{
		return false;
	}
	key_t keyShmKey = ftok((m_szShmName + ".shm").c_str(), 'a');
	if (keyShmKey == -1)
	{
		return false;
	}

	m_hShmId = shmget(keyShmKey, m_qwSize, SHM_R | SHM_W | IPC_CREAT);
	if (m_hShmId == -1)
	{
		return false;
	}

	m_pData = shmat(m_hShmId, pAddr, 0);
	if (m_pData == (void*)-1)
	{
		shmctl(m_hShmId, IPC_RMID, NULL);
		return false;
	}
	assert(m_pData == mmap(m_pData, m_qwSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fHandle, 0));
#endif // _WIN32
	return true;
}

#ifdef _WIN32
bool CShareMem::ResizeFile(HANDLE hFile)
{
	LARGE_INTEGER qwFileSize;
	GetFileSizeEx(hFile, &qwFileSize);

	if (qwFileSize.QuadPart < (long long)m_qwSize)
	{
		HANDLE hTemp = hFile;
		SetFilePointerEx(hTemp, qwFileSize, 0, FILE_BEGIN);
		char pBuff[4096] = { 0 };
		for (long long i = m_qwSize - qwFileSize.QuadPart; i > 0; i -= 4096)
		{
			OVERLAPPED qwOut;
			WriteFileEx(hTemp, pBuff, (DWORD)(i > 4096 ? 4096 : i), &qwOut, 0);
		}
	}

	return true;
}
#else
#endif // _WIN32



