#include "replace_dynamic_library.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#endif	//!_WIN32

#include <string.h>

int ReplaceDynamicLibrary::operator ()(const std::string& szDynamicLibraryName, const std::string& szFunctionName)
{
#ifdef _WIN32
	HMODULE hDll = LoadLibrary(szDynamicLibraryName.c_str());
	void* pNewFunc = (void*)GetProcAddress(hDll, szFunctionName.c_str());

	void* pOldFunc = (void*)GetProcAddress(0, szFunctionName.c_str());

	DWORD dwOldFlag = 0;
	VirtualProtectEx(GetCurrentProcess(), (void*)((long long)pOldFunc - 5), 5 + 2, PAGE_EXECUTE_READWRITE, &dwOldFlag); 
#else
	void* pHandleSo = dlopen(szDynamicLibraryName.c_str(), RTLD_NOW);
	if (NULL == pHandleSo)
	{
		printf("dlopen - %\n\r", dlerror()); 
		return 1;
	}

	void* pOldAddr = dlsym(NULL, szFunctionName.c_str());
	if (NULL == pOldAddr)
	{
		printf("dlopen - %s\n\r", dlerror()); 
		return 1;
	}
	
	void* pNewAddr = dlsym(pHandleSo, szFunctionName.c_str());
	if (NULL == pOldAddr)
	{
		printf("dlopen - %s\r", dlerror()); 
		return 1;
	}

	int dwPageSize = sysconf(_SC_PAGE_SIZE);
	int dwPageMask = ~(dwPageSize - 1);
	// int dwPageNum = (dwPageSize - (pOldAddr & ~dwPageMask) >= size) ? 1 : 2;
	int dwPageNum = 2;
	if (mprotect((void *)((uintptr_t)pOldAddr & dwPageMask), dwPageNum * dwPageSize, PROT_READ | PROT_WRITE | PROT_EXEC))
	{
		return 1;
	}
	
	/*
	* Translate the following instructions 
	* 
	* mov $new_func_entry, %rax 
	* jmp %rax 
	* 
	* into machine code 
	* 
	* 48 b8 xx xx xx xx xx xx xx xx 
	* ff e0 
	*/
	memset(pOldAddr, 0x48, 1);
	memset((void*)((uintptr_t)pOldAddr + 1), 0xb8, 1); 
	memcpy((void*)((uintptr_t)pOldAddr + 2), &pNewAddr, 8); 
	memset((void*)((uintptr_t)pOldAddr + 10), 0xff, 1);
	memset((void*)((uintptr_t)pOldAddr + 11), 0xe0, 1);

	if (mprotect((void *)((uintptr_t)pOldAddr & dwPageMask), dwPageNum * dwPageSize, PROT_READ | PROT_EXEC))
	{
		return 1;
	}

	return 0;
#endif	//!_WIN32

	return 1;
}

