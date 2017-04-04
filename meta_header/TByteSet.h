#ifndef __TByteSet_h__
#define __TByteSet_h__

#include <string.h>

//这个类型的对象根本不会被创建成功
#define EnumByteSet(EnumName) \
template<EnumName>\
class T##EnumName##ByteSet\
{\
private:\
	T##EnumName##ByteSet();\
};\
template<>\
class T##EnumName##ByteSet<EnumName##Count>\
{\
public:\
	T##EnumName##ByteSet(){memset(m_pData, 0, sizeof(m_pData));}\
	virtual ~T##EnumName##ByteSet(){}\
	bool SetByte(EnumName e, bool bValue)\
	{\
		if (e >= EnumName##Count)\
		{\
			return false;\
		}\
		if (bValue)\
		{\
			m_pData[e / 8] |= (1 << (e % 8));\
		}\
		else\
		{\
			m_pData[e / 8] &= ~(1 << (e % 8));\
		}\
		return true;\
	}\
	bool GetByte(EnumName e)\
	{\
		if (e >= EnumName##Count)\
		{\
			return false;\
		}\
		return (m_pData[e / 8] & (1 << (e % 8))) != 0;\
	}\
private:\
	char m_pData[EnumName##Count / 8 + 1];\
};\
typedef T##EnumName##ByteSet<EnumName##Count> EnumName##ByteSet;

#define BeginEnumByteSet(EnumName) \
enum EnumName

#define EndEnumByteSet(EnumName) \
EnumName##Count,

#define EndEnumByteSetWithCount(EnumName, NUM) \
EnumName##Count = NUM,


#endif // !__TByteSet_h__
