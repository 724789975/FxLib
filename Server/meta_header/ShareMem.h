#ifndef __ShareMem_H__
#define __ShareMem_H__

#include <new>
#include <stdio.h>
#include <string>

#ifdef WIN32
#include <windows.h>
#endif // WIN32


class CShareMem
{
public:
	CShareMem(std::string szName, unsigned long long qwSize);
	~CShareMem();
	void* m_pData;
	bool m_bAlreadyThere;

public:

	void DeleteShareMem();

	bool IsAlreadyThere()
	{
		return m_bAlreadyThere;
	}

	// bCreated 是否创建了新的共享内存
	bool Init(bool& bCreated);

private:
	bool IsExist();
	bool Open();
	bool Create();
#ifdef WIN32
	bool ResizeFile(HANDLE h_File);
#else
#endif // WIN32

	std::string m_szShmName;
	unsigned long long m_qwSize;

#ifdef WIN32
	HANDLE
#else
	int
#endif // WIN32
		m_hShmId;

};

#endif // !__ShareMem_H__
