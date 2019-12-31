#ifndef __TBitSet_h__
#define __TBitSet_h__

#include <string.h>

//这个类型的对象根本不会被创建成功
#define EnumBitSet(EnumName) \
template<EnumName>\
class T##EnumName##BitSet\
{\
private:\
	T##EnumName##BitSet();\
};\
template<>\
class T##EnumName##BitSet<EnumName##Count>\
{\
public:\
	T##EnumName##BitSet(){memset(m_pData, 0, sizeof(m_pData));}\
	virtual ~T##EnumName##BitSet(){}\
	bool SetBit(EnumName e, bool bValue)\
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
	bool GetBit(EnumName e)\
	{\
		if (e >= EnumName##Count)\
		{\
			return false;\
		}\
		return (m_pData[e / 8] & (1 << (e % 8))) != 0;\
	}\
	void Reset()\
	{\
		memset(m_pData, 0, sizeof(m_pData));\
	}\
private:\
	char m_pData[EnumName##Count / 8 + 1];\
};\
typedef T##EnumName##BitSet<EnumName##Count> EnumName##BitSet;

#define BeginEnumBitSet(EnumName) \
enum EnumName

#define EndEnumBitSet(EnumName) \
EnumName##Count,

#define EndEnumBitSetWithCount(EnumName, NUM) \
EnumName##Count = NUM,


#endif // !__TBitSet_h__
