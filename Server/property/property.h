#ifndef __Property_H__
#define __Property_H__

#include "integral_constant.h"
#include "derive_list.h"
#include "args_num.h"
#include "strhelper.h"

#include <sstream>

template<typename T, const char*(szName)()>
class Property
{
public:
	typedef T TypeValue;
	Property()
		: m_bChanged(false)
	{}
	Property(const T& t)
		: m_bChanged(false)
		, m_tValue(t)
	{}

	bool Changed()const { return m_bChanged; }
	void Changed(bool bChanged)
	{
		m_bChanged = bChanged;
	}

	T& Value()
	{
		return m_tValue;
	}

	Property<T, szName>& operator = (const T& refT)
	{
		m_tValue = refT;
		m_bChanged = true;
		return *this;
	}

	const T& Value()const
	{
		return m_tValue;
	}

	const char* Name()
	{
		return szName();
	}

private:
	bool m_bChanged;
	T m_tValue;
};

#define CommonPropertyDeclare(Type, Name) \
class Name \
{\
public:\
	static const char* __##Name () { return #Name; }\
	Type & Get##Name () {return m_oProperty.Value();}\
	void Set##Name (const Type& value) { m_oProperty = value; }\
	Property<Type, __##Name>& Data() { return m_oProperty; }\
private:\
	Property<Type, __##Name> m_oProperty;\
};

#define HasOnChange(C, F)\
template<typename T>\
struct C##Has##F {\
	template<typename U, void (U::*)()> struct HELPS;\
	template<typename U> static char Check(HELPS<U, &U::F>*);\
	template<typename U> static int Check(...);\
	const static bool Has = sizeof(Check<T>(0)) == sizeof(char);\
};

#define PropertyDefine(C, Type, Name) \
CommonPropertyDeclare(Type, Name)\
HasOnChange(C, On##Name##Change)\
template<typename T> \
void Name##Changed(T* pT, true_type t){ pT->On##Name##Change(); }\
template<typename T> \
void Name##Changed(T* pT, false_type f){}\
Type & Get##Name (){return m_oPropertys.Get##Name();}\
void Set##Name (const Type& value){ m_bChanged = true; return m_oPropertys.Set##Name(value); } \
void Set##Name (const Type& value, std::ostream& refOstream)\
{\
	refOstream << __FUNCTION__ << " old value : " << Get##Name();\
	m_oPropertys.Set##Name(value);\
	refOstream << ", new value : " << Get##Name();\
	Name##Changed(this, integral_constant<bool, C##Has##On##Name##Change<C>::Has>()); \
}

#define GetPropertyRdedisStringDeclare	\
template<int dwIndex> \
void GetRedisStringData(std::stringstream& refOstream); \
template<> \
void GetRedisStringData<DL::Length<Propertys>::Value>(std::stringstream& refOstream);

#define GetPropertyRdedisStringDefine \
template<int dwIndex> \
void Table::GetRedisStringData(std::stringstream& refOstream) \
{\
	typename DL::TypeAt<Propertys, dwIndex>::Result& refData = m_oPropertys; \
	if (refData.Data().Changed())\
	{\
		refOstream << "\"" << refData.Data().Name() << "\" " << ToRedisString(refData.Data().Value()); \
	}\
	GetRedisStringData<dwIndex + 1>(refOstream); \
}\
template<> \
void Table::GetRedisStringData<DL::Length<Table::Propertys>::Value>(std::stringstream& refOstream) \
{}

#define ProoertyTableName(Name) \
const char* RedisTableName(){ return #Name; }

class Table
{
public:
	ProoertyTableName(table);
	PropertyDefine(Table, int, RoleId);
	PropertyDefine(Table, double, TeamId);

	typedef DERIDELIST_2(RoleId, TeamId) Propertys;

	GetPropertyRdedisStringDeclare;

	//void OnRoleIdChange() {}
	void OnTeamIdChange()
	{
		int a = 0;
		++a;
	}

	const char* GetRedisStringData()
	{
		if (!m_bChanged) { return ""; }
		std::stringstream ssData;
		ssData << "HMSET " << RedisTableName() << "_" << GetRoleId() << " ";
		GetRedisStringData<0>(ssData);
		const char* szData = ssDat.str().c_str();
		return szData;
	}

protected:
private:

	Propertys m_oPropertys;

	bool m_bChanged;
};

GetPropertyRdedisStringDefine;


#endif // !__Property_H__
