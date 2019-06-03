#ifndef __STRHELPER_H_2009_0824__
#define __STRHELPER_H_2009_0824__

#include <string>
#include <stdlib.h>
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
	size_t dwLen = szGBK.size();
	char* outbuf = (char*)malloc(dwLen * 4);
	memset(outbuf, 0, dwLen * 4);
	const char* szIn = szGBK.c_str();
	char* szOut = outbuf;
	size_t dwOutLen = dwLen * 4;
	iconv(cd, (char**)&szIn, &dwLen, &outbuf, &dwOutLen);
	dwOutLen = strlen(outbuf);
	std::string strOutUTF8 = szOut;
	free(outbuf);
	iconv_close(cd);
	return strOutUTF8;
#endif // WIN32

}

template<typename T>
inline const char* Value2String(const T& c)
{
	static char szBuff[512] = { 0 };

	memcpy(szBuff, &c, 512);
	szBuff[511] = 0;
	return szBuff;
}

template<>
inline const char* Value2String<char>(const char& c)
{
	static char szBuff[2] = { 0 };
	szBuff[0] = c;
	szBuff[1] = 0;
	return szBuff;
}

template<>
inline const char* Value2String<unsigned char>(const unsigned char& c)
{
	static char szBuff[2] = { 0 };
	szBuff[0] = c;
	szBuff[1] = 0;
	return szBuff;
}

template<>
inline const char* Value2String<short>(const short& wSrc)
{
	static char szBuff[8] = { 0 };
	sprintf(szBuff, "%d", wSrc);
	return szBuff;
}

template<>
inline const char* Value2String<unsigned short>(const unsigned short& wSrc)
{
	static char szBuff[8] = { 0 };
	sprintf(szBuff, "%d", wSrc);
	return szBuff;
}

template<>
inline const char* Value2String<int>(const int& dwSrc)
{
	static char szBuff[8] = { 0 };
	sprintf(szBuff, "%d", dwSrc);
	return szBuff;
}

template<>
inline const char* Value2String<unsigned int>(const unsigned int& dwSrc)
{
	static char szBuff[32] = { 0 };
	sprintf(szBuff, "%d", dwSrc);
	return szBuff;
}

template<>
inline const char* Value2String<float>(const float& dwSrc)
{
	static char szBuff[32] = { 0 };
	sprintf(szBuff, "%f", dwSrc);
	return szBuff;
}

template<>
inline const char* Value2String<double>(const double& qwSrc)
{
	static char szBuff[64] = { 0 };
	sprintf(szBuff, "%g", qwSrc);
	return szBuff;
}

template<>
inline const char* Value2String<long long>(const long long& qwSrc)
{
	static char szBuff[64] = { 0 };
	sprintf(szBuff, "%lld", qwSrc);
	return szBuff;
}

template<>
inline const char* Value2String<unsigned long long> (const unsigned long long& qwSrc)
{
	static char szBuff[64] = { 0 };
	sprintf(szBuff, "%llu", qwSrc);
	return szBuff;
}

template<>
inline const char* Value2String<std::string>(const std::string& qwSrc)
{
	return qwSrc.c_str();
}

template<typename T>
inline const char* Value2RedisString(T t)
{
	return Value2String(t);
}

template<>
inline const char* Value2RedisString<>(const std::string& szData)
{
	std::string sz = "\"";
	sz += szData;
	sz += "\"";
	return sz.c_str();
}

template<>
inline const char* Value2RedisString<>(const char* szData)
{
	std::string sz = "\"";
	sz += szData;
	sz += "\"";
	return sz.c_str();
}

template<typename T>
inline void String2Value(const char* szData, T& refValue)
{
	memcpy(&refValue, szData, sizeof(T));
}

template<>
inline void String2Value<char>(const char* szData, char& refValue)
{
	refValue = *szData;
}

template<>
inline void String2Value<short>(const char* szData, short& refValue)
{
	refValue = atoi(szData);
}

template<>
inline void String2Value<unsigned short>(const char* szData, unsigned short& refValue)
{
	refValue = atoi(szData);
}

template<>
inline void String2Value<int>(const char* szData, int& refValue)
{
	refValue = atoi(szData);
}

template<>
inline void String2Value<unsigned int>(const char* szData, unsigned int& refValue)
{
	refValue = atoi(szData);
}

template<>
inline void String2Value<float>(const char* szData, float& refValue)
{
	refValue = (float)atof(szData);
}

template<>
inline void String2Value<long long>(const char* szData, long long& refValue)
{
	refValue = atoll(szData);
}

template<>
inline void String2Value<unsigned long long>(const char* szData, unsigned long long& refValue)
{
	refValue = atoll(szData);
}

template<>
inline void String2Value<double>(const char* szData, double& refValue)
{
	refValue = atof(szData);
}





#endif  // __STRHELPER_H_2009_0824__
