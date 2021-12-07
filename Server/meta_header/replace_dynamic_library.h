#ifndef __REPLACE_DYNAMIC_LIBRARY_H__
#define __REPLACE_DYNAMIC_LIBRARY_H__

#include <string>

class ReplaceDynamicLibrary
{
public:
    int operator ()(const std::string& szDynamicLibraryName, const std::string& szFunctionName);
	int operator ()(void* pOldAddr, const std::string& szDynamicLibraryName, const std::string& szFunctionName);
	int HotPatchFunction(void* pOldAddr, void* pNewAddr);
private:
};

#endif  //!__REPLACE_DYNAMIC_LIBRARY_H__