#ifndef __REPLACE_DYNAMIC_LIBRARY_H__
#define __REPLACE_DYNAMIC_LIBRARY_H__

#include <string>

class ReplaceDynamicLibrary
{
public:
    int operator ()(const std::string& szDynamicLibraryName, const std::string& szFunctionName);
};

#endif  //!__REPLACE_DYNAMIC_LIBRARY_H__