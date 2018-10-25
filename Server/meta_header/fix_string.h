#ifndef __FixString_H__
#define __FixString_H__
#include <string>

template<unsigned int dwStringLen>
class FixString
{
public:
	FixString()
	{}
	FixString(const char* szString)
	{
		memcpy(m_szString, szString, dwStringLen);
		m_szString[dwStringLen - 1] = 0;
	}
	FixString(const std::string& szString)
	{
		memcpy(m_szString, szString.c_str(), dwStringLen);
		m_szString[dwStringLen - 1] = 0;
	}

	FixString<dwStringLen>& operator = (const char* szString)
	{
		memcpy(m_szString, szString, dwStringLen);
		m_szString[dwStringLen - 1] = 0;
		return *this;
	}

	FixString<dwStringLen>& operator = (const std::string& szString)
	{
		memcpy(m_szString, szString.c_str(), dwStringLen);
		m_szString[dwStringLen - 1] = 0;
		return *this;
	}

	bool operator < (const FixString<dwStringLen>& refFixString) const
	{
		return strcmp(m_szString, refFixString.m_szString) < 0;
	}

	void Clear()
	{
		memset(m_szString, 0, dwStringLen);
	}

	const char* String()const
	{
		return m_szString;
	}
	char* String()
	{
		return m_szString;
	}
protected:
private:

	char m_szString[dwStringLen];
};

#endif // !__FixString_H__

