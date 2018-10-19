#include "ShareMem.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#endif

#include <assert.h>

CShareMem::CShareMem(std::string szName, unsigned long long qwSize)
	: m_pData(NULL)
	, m_szShmName(szName)
	, m_qwSize(qwSize)
#ifdef WIN32
	, m_hShmId(INVALID_HANDLE_VALUE)
#else
	, m_hShmId(0XFFFFFFFF)
#endif // WIN32

{
}

CShareMem::~CShareMem()
{
}

bool CShareMem::Init(bool& bCreated)
{
	bCreated = false;
	if (!Open())
	{
		bCreated = true;
		return Create();
	}
	return true;
}

void CShareMem::DeleteShareMem()
{
#ifdef WIN32
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
#ifdef WIN32
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
#endif // WIN32
	return false;
}

bool CShareMem::Open()
{
	assert(m_pData == NULL);
#ifdef WIN32
	HANDLE hFile = CreateFile((m_szShmName + ".shm").c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	ResizeFile(hFile);

	m_hShmId = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
		m_qwSize, m_szShmName.c_str());
	if (m_hShmId == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	m_hShmId = OpenFileMapping(FILE_MAP_READ | FILE_SHARE_WRITE, FALSE, m_szShmName.c_str());
	if (m_hShmId == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	m_pData = MapViewOfFile(m_hShmId, FILE_MAP_READ | FILE_SHARE_WRITE, 0, 0, 0);
	if (m_pData == NULL)
	{
		CloseHandle(hFile);
		return false;
	}
#else
	//int fHandle = open((m_szShmName + ".shm").c_str(), O_RDWR, 0777);
	//if (fHandle == -1)
	//{
	//	return false;
	//}
	key_t keyShmKey = ftok(m_szShmName.c_str(), 'a');
	if (keyShmKey == -1)
	{
		return false;
	}

	m_hShmId = shmget(keyShmKey, 0, SHM_R | SHM_W);
	if (m_hShmId == -1)
	{
		return false;
	}

	m_pData = shmat(m_hShmId, NULL, 0);
	if (m_pData == (void*)-1)
	{
		return false;
	}
#endif // WIN32
	return true;
}

bool CShareMem::Create()
{
	assert(m_pData == NULL);
#ifdef WIN32
	HANDLE hFile = CreateFile((m_szShmName + ".shm").c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, CREATE_NEW, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	ResizeFile(hFile);

	m_hShmId = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
		m_qwSize, m_szShmName.c_str());
	if (m_hShmId == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	m_pData = MapViewOfFile(m_hShmId, FILE_MAP_READ | FILE_SHARE_WRITE, 0, 0, 0);
	if (m_pData == NULL)
	{
		CloseHandle(hFile);
		return false;
	}
#else
	//int fHandle = open((m_szShmName + ".shm").c_str(), O_RDWR, 0777);
	//if (fHandle == -1)
	//{
	//	return false;
	//}
	key_t keyShmKey = ftok(m_szShmName.c_str(), 'a');
	if (keyShmKey == -1)
	{
		return false;
	}

	m_hShmId = shmget(keyShmKey, 0, SHM_R | SHM_W | IPC_CREAT);
	if (m_hShmId == -1)
	{
		return false;
	}

	m_pData = shmat(m_hShmId, NULL, 0);
	if (m_pData == (void*)-1)
	{
		shmctl(m_hShmId, IPC_RMID, NULL);
		return false;
	}
#endif // WIN32
	return true;
}

#ifdef WIN32
bool CShareMem::ResizeFile(HANDLE hFile)
{
	LARGE_INTEGER qwFileSize;
	GetFileSizeEx(hFile, &qwFileSize);

	if (qwFileSize.QuadPart < m_qwSize)
	{
		HANDLE hTemp = hFile;
		SetFilePointerEx(hTemp, qwFileSize, 0, FILE_BEGIN);
		char pBuff[4096] = { 0 };
		for (long long i = m_qwSize - qwFileSize.QuadPart; i > 0; i -= 4096)
		{
			OVERLAPPED qwOut;
			WriteFileEx(hTemp, pBuff, i > 4096 ? 4096 : i, &qwOut, 0);
		}
	}

	return true;
}
#else
#endif // WIN32



