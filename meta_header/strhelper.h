#ifndef __STRHELPER_H_2009_0824__
#define __STRHELPER_H_2009_0824__

#ifdef WIN32
#else
#include <string.h>
#endif

inline void _StrSafeCopy(char *pszDest, const char *pszSrc, size_t nLen) throw()
{
	if(NULL == pszSrc)
	{
		pszDest[0] = '\0';
		return;
	}

#ifdef WIN32
	lstrcpyn(pszDest, pszSrc, (INT32)nLen);
#else
	size_t nSrcLen = strnlen(pszSrc, nLen-1);
	memcpy(pszDest, pszSrc, nSrcLen+1);
	pszDest[nLen-1] = '\0';
#endif
}

#ifdef WIN32
template <class T>
inline void StrSafeCopy(T& Destination, const char* Source) throw()
{
    (static_cast<char[sizeof(Destination)]>(Destination));

    _StrSafeCopy(Destination, Source, sizeof(Destination));
}

template <class T>
inline void ZeroString(T& Destination) throw()
{
    (static_cast<char[sizeof(Destination)]>(Destination));
    Destination[0] = '\0';
    Destination[sizeof(Destination)-1] = '\0';
}

template <class T>
inline void SafeSprintf(T& Destination, const char *format, ...) throw()
{
    (static_cast<char[sizeof(Destination)]>(Destination));
    
    va_list args;
    va_start(args,format);
    _vsnprintf(Destination, sizeof(Destination)-1, format, args);
    va_end(args);
    Destination[sizeof(Destination)-1] = '\0';
}

#else

#define StrSafeCopy(Destination, Source) \
	_StrSafeCopy(Destination, Source, sizeof(Destination));

#define ZeroString(Destination)\
	Destination[0] = '\0';\
	Destination[sizeof(Destination)-1] = '\0';

#endif

#ifdef WIN32
inline size_t StrNLen(const char *s, size_t maxlen)
{
	size_t i;
	const char *ptr = s;

	for(i = 0; i < maxlen; i++)
	{
		if('\0' == *ptr)
		{
			return i;
		}
		ptr++;
	}

	return maxlen;
}
#else
inline size_t StrNLen(const char *s, size_t maxlen)
{
	return strnlen(s, maxlen);
}
#endif

#endif  // __STRHELPER_H_2009_0824__
