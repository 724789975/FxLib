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

public:

	void DeleteShareMem();

	// bCreated 是否创建了新的共享内存
	bool Init(bool& bCreated, void* pAddr = NULL);

	template<typename T>
	T* GetMemory()
	{
		return reinterpret_cast<T*>(m_pData);
	}
private:
	bool IsExist();
	bool Open(void* pAddr);
	bool Create(void* pAddr);
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

	void* m_pData;
};

#endif // !__ShareMem_H__
