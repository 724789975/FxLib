/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tencent is pleased to support the open source community by making behaviac available.
//
// Copyright (C) 2015-2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at http://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DEFINES_H__
#define __DEFINES_H__

///////////////////////////////////////////////////////////////////////////////
#if _MSC_VER
//#if !defined(_CRT_SECURE_NO_WARNINGS)
//#define _CRT_SECURE_NO_WARNINGS
//#endif

#define CCDEFINE_MSVC		1
#define CDEFINE_BIGENDIAN			1

#if _MSC_VER >= 1900
#define CCDEFINE_MSVC2015 1
#define CDEFINE_NAME "vs2015"
#elif _MSC_VER >= 1800
#define CCDEFINE_MSVC2013 1
#define CDEFINE_NAME "vs2013"
#elif _MSC_VER >= 1600
#define CCDEFINE_MSVC2010 1
#define CDEFINE_NAME "vs2010"
#elif _MSC_VER >= 1500
#define CCDEFINE_MSVC2008 1
#define CDEFINE_NAME "vs2008"
#define nullptr 0
#elif _MSC_VER >= 1400
#define CCDEFINE_MSVC2005 1
#define CDEFINE_NAME "vs2005"
#define nullptr 0
#elif _MSC_VER >= 1310
#define CCDEFINE_MSVC2003 1
#define CDEFINE_NAME "vs2003"
#define nullptr 0
#else
#error Requires Visual C++ 2003 or above
#endif//_MSC_VER

#if _WIN64
#define CDEFINE_64BITS	1
#endif

#elif __APPLE_CC__
#include "TargetConditionals.h"

#define CCDEFINE_APPLE 1
//#define CCDEFINE_BIGENDIAN			1
#define CCDEFINE_NAME "gcc-apple"

#if defined(__LP64__)
#define CCDEFINE_64BITS	1
#endif

#if TARGET_OS_IPHONE
#define CCDEFINE_APPLE_IPHONE	1
#endif

#elif __ANDROID__ || ANDROID
#define CCDEFINE_ANDROID 1
//#define CCDEFINE_BIGENDIAN			1
#define CCDEFINE_NAME "gcc-android"

#if defined(__LP64__)
#define CCDEFINE_64BITS	1
#endif

#ifndef CCDEFINE_ANDROID_VER
#define CCDEFINE_ANDROID_VER 9
#endif

#elif __CYGWIN__ || __MINGW32__ || __MINGW64__
#define CCDEFINE_GCC_CYGWIN 1
//#define CCDEFINE_BIGENDIAN			1
#define CCDEFINE_NAME "gcc-cygwin"
#if __GNUC__ < 4 || __GNUC_MINOR < 6
#define nullptr 0
#endif

#if defined(__LP64__)
#define CCDEFINE_64BITS	1
#endif

#elif defined(__linux__)
//#define CCDEFINE_BIGENDIAN			1
#define CCDEFINE_GCC_LINUX 1
#define CCDEFINE_NAME "gcc-linux"

#if defined(__LP64__)
#define CCDEFINE_64BITS	1
#endif
#else
#error Unsupported C++ compiler

//#define CCDEFINE_BIGENDIAN			1

#endif // _MSC_VER

//#if !defined(CCDEFINE_BIGENDIAN)
//#error please define CCDEFINE_BIGENDIAN 
//#endif

///////////////////////////////////////////////////////////////////////////////
// Visual C++
#if CCDEFINE_MSVC
#define FORCEINLINE __inline
#define FORCENOINLINE _declspec(noinline)

#if _MSC_VER >= 1400
#define RESTRICT __restrict
#else
#define RESTRICT
#endif//#if _MSC_VER >= 1400

#define ALIGN_PREFIX(alignment)				__declspec(align(alignment))
#define ALIGN_SUFFIX(alignment)

#ifdef DLL
#define DLL_ENTRY_IMPORT						__declspec(dllimport)
#define DLL_ENTRY_EXPORT						__declspec(dllexport)
#else
#define DLL_ENTRY_IMPORT
#define DLL_ENTRY_EXPORT
#endif//DLL

#elif CCDEFINE_APPLE
#define FORCEINLINE inline

#define RESTRICT __restrict__

#define ALIGN_PREFIX(alignment)
#define ALIGN_SUFFIX(alignment)				__attribute__ ((aligned(n)))

#ifdef DLL
#	define DLL_ENTRY_IMPORT
#	define DLL_ENTRY_EXPORT					__attribute__((visibility("default")))
#else
#	define DLL_ENTRY_IMPORT
#	define DLL_ENTRY_EXPORT
#endif//DLL

#else
//GCC
//#pragma GCC diagnostic ignored "-Wmissing-braces"

#define FORCEINLINE inline

#define RESTRICT __restrict__

//#define ALIGN_PREFIX(alignment)
//#define ALIGN_SUFFIX(alignment)				__attribute__ ((aligned(n)))

#ifdef DLL
#	define DLL_ENTRY_IMPORT
#	define DLL_ENTRY_EXPORT					__attribute__((visibility("default")))
#else
#	define DLL_ENTRY_IMPORT
#	define DLL_ENTRY_EXPORT
#endif//DLL

#endif // CCDEFINE_GCC_CYGWIN

#ifdef BEHAVIACDLL_EXPORTS
#	define API DLL_ENTRY_EXPORT
#else
#	define API DLL_ENTRY_IMPORT
#endif//BEHAVIACDLL_EXPORTS

#if CCDEFINE_MSVC
////warning C4275: non dll-interface class 'stdext::exception' used as base for dll-interface class 'std::bad_cast'
//#pragma warning(disable : 4275)
//
////warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
//#pragma warning(disable : 4530)
//
////warning C4251: 'behaviac::VariableRegistry::m_proxyHolders' : class 'behaviac::vector<T>' needs to have dll-interface to be used
////by clients of class 'behaviac::VariableRegistry'
//#pragma warning(disable : 4251)
//
////unreferenced formal parameter
//#pragma warning(disable : 4100)
//
//#pragma warning(disable : 4127) // conditional expression is constant
//
////warning C4702: unreachable code
//#pragma warning(disable : 4702)

//'strcpy': This function or variable may be unsafe.
#pragma warning(disable : 4996)

#endif//CCDEFINE_MSVC

#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

#define ARRAY_LENGTH(s) (sizeof(s) / sizeof(s[0]))

// to remove unused variable warning
#define UNUSED_VAR(var)         ((void) &var)

#if CCDEFINE_MSVC
# define UNUSED
# define ALIAS
#else
# define UNUSED __attribute__((unused))
# define ALIAS __attribute__((__may_alias__))
#endif

#define _OFFSETOF_BASE_ 0x01000000
#define OFFSETOF(TYPE, MEMBER) (size_t)((unsigned char*)(&(((TYPE*)_OFFSETOF_BASE_)->MEMBER)) - (unsigned char*)(TYPE*)_OFFSETOF_BASE_)

//#define OFFSETOF_POD(TYPE, MEMBER) offsetof(TYPE, MEMBER)
#define OFFSETOF_POD(TYPE, MEMBER) OFFSETOF(TYPE, MEMBER)

#define string_cpy(d, s) strcpy(d, s)
#define string_ncpy(d, s, n) strncpy(d, s, n)
#define string_ncpy_s(d, s, n) strncpy(d, s, n - 1); d[n - 1] = '\0';
#define string_cat(d, s) strcat(d, s)
#define string_cmp(d, s) strcmp(d, s)

#if CCDEFINE_MSVC
#define string_cpy_s(d, n, s) strcpy_s(d, n, s)
#define string_cat_s(d, n, s) strcat_s(d, n, s)
#define string_icmp _stricmp
#define string_nicmp _strnicmp
#define string_snprintf _snprintf_s
#define string_vnprintf vsnprintf
#define string_vnwprintf _vsnwprintf_s
#define string_sprintf sprintf_s
#define Access _access
//#define string_sprintf(s, fmt, ...) ASSERT(ARRAY_LENGTH(s) > 0); _snprintf(s, ARRAY_LENGTH(s), fmt, __VA_ARGS__); s[ARRAY_LENGTH(s) - 1] = '\0';
#elif CCDEFINE_APPLE || CCDEFINE_ANDROID || CCDEFINE_GCC_LINUX || CCDEFINE_GCC_CYGWIN
#define string_cpy_s(d, n, s) strcpy_s(d, s)
#define string_cat_s(d, n, s) strcat(d, s)
#define string_icmp strcasecmp
#define string_nicmp strncasecmp
#define string_snprintf snprintf
#define string_vnprintf vsnprintf
#define string_vnwprintf vswprintf
#define string_sprintf snprintf
#define Access access
//#define string_sprintf(s, fmt, ...) ASSERT(ARRAY_LENGTH(s) > 0); snprintf(s, ARRAY_LENGTH(s), fmt, __VA_ARGS__); s[ARRAY_LENGTH(s) - 1] = '\0'
#else
#define string_cpy_s(d, n, s) strcpy_s(d, s)
#define string_cat_s(d, n, s) strcat(d, s)
#define string_icmp stricmp
#define string_nicmp strnicmp
#define string_snprintf snprintf
#define string_vnprintf vsnprintf
#define string_vnwprintf vswprintf
#define string_sprintf snprintf
#define Access access
//#define string_sprintf(s, fmt, ...) ASSERT(ARRAY_LENGTH(s) > 0); snprintf(s, ARRAY_LENGTH(s), fmt, __VA_ARGS__); s[ARRAY_LENGTH(s) - 1] = '\0'
#endif//CCDEFINE_MSVC

#define HW_MEM_ALIGN								16

#define SHADOW_SIZE_SPIN_LOCK						24
#define SHADOW_SIZE_ADAPTIVE_LOCK					40

#define CFG_THREADNAME_MAXLENGTH                   64
#define CFG_FILENAME_MAXLENGTH                     255
#define CFG_CLASSNAME_MAXLENGTH                    200
#define CFG_SETEXENAME_BUF_SIZE					256

#endif // __DEFINES_H__
