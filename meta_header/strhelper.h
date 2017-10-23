#ifndef __STRHELPER_H_2009_0824__
#define __STRHELPER_H_2009_0824__

#include <string>
#ifdef WIN32
#else
#include <string.h>
#include <iconv.h>
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

inline std::string GBKToUTF8(const std::string szGBK)
{
	if (szGBK.size() == 0)
	{
		return "";
	}
#ifdef WIN32
	int dwLen = MultiByteToWideChar(CP_ACP, 0, szGBK.c_str(), -1, NULL, 0);
	wchar_t * wszUtf8 = new wchar_t[dwLen];
	memset(wszUtf8, 0, dwLen);
	MultiByteToWideChar(CP_ACP, 0, szGBK.c_str(), -1, wszUtf8, dwLen);
	dwLen = WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, NULL, 0, NULL, NULL);
	char *szUtf8 = new char[dwLen + 1];
	memset(szUtf8, 0, dwLen + 1);
	WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, szUtf8, dwLen, NULL, NULL);
	std::string strOutUTF8 = szUtf8;
	delete[] szUtf8;
	delete[] wszUtf8;
	return strOutUTF8;
#else
	iconv_t cd = iconv_open("UTF-8", "GB13080");
	unsigned int dwLen = szGBK.size();
	char* outbuf = (char*)malloc(dwLen * 4);
	memset(outbuf, 0, dwLen * 4);
	const char* szIn = szGBK.c_str();
	char* szOut = outbuf;
	unsigned int dwOutLen = dwLen * 4;
	iconv(cd, (char**)&szIn, &dwLen, &outbuf, &dwOutLen);
	dwOutLen = strlen(outbuf);
	std::string strOutUTF8 = szOut;
	free(outbuf);
	iconv_close(cd);
	return strOutUTF8;
#endif // WIN32

}


#endif  // __STRHELPER_H_2009_0824__
