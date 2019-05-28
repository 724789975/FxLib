#ifndef __Property_H__
#define __Property_H__

#include "derive_list.h"
#include "args_num.h"
#include "strhelper.h"
#include "fix_string.h"
#include "meta/meta.h"

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
	typedef Type TypeValue;\
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
void Name##Changed(T* pT, Meta::TrueType t){ pT->On##Name##Change(); }\
template<typename T> \
void Name##Changed(T* pT, Meta::FalseType f){}\
Type & Get##Name (){return m_oPropertys.Get##Name();}\
void Set##Name (const Type& value){ m_bChanged = true; return m_oPropertys.Set##Name(value); } \
void Set##Name (const Type& value, std::ostream& refOstream)\
{\
	refOstream << __FUNCTION__ << " old value : " << Get##Name();\
	Set##Name(value);\
	refOstream << ", new value : " << Get##Name();\
	Name##Changed(this, Meta::BooleanType<C##Has##On##Name##Change<C>::Has>()); \
}

#define PrimaryPropertyDefine(C, Type, Name) \
PropertyDefine(C, Type, Name) \
Type & GetPrimaryKey() { return Get##Name (); }

#define PropertyRdedisStringDeclare	\
std::string GetRedisSaveString(); \
template<int dwIndex> \
void GetRedisSaveString(std::ostream& refOstream);\
std::string GetRedisFetchString(); \
template<int dwIndex> \
void GetRedisFetchString(std::ostream& refOstream);

#define PropertyRdedisStringDefine \
std::string Table::GetRedisSaveString()\
{\
	if (!m_bChanged) { return ""; }\
	std::stringstream ssData;\
	ssData << "HMSET " << RedisTableName() << "_" << GetPrimaryKey();\
	GetRedisSaveString<0>(ssData);\
	return ssData.str();\
}\
template<int dwIndex> \
void Table::GetRedisSaveString(std::ostream& refOstream) \
{\
	typename DL::TypeAt<Propertys, dwIndex>::Result& refData = m_oPropertys; \
	if (refData.Data().Changed())\
	{\
		refOstream << " \"" << refData.Data().Name() << "\" " << Value2RedisString(refData.Data().Value()); \
	}\
	GetRedisSaveString<dwIndex + 1>(refOstream); \
}\
template<> \
void Table::GetRedisSaveString<DL::Length<Table::Propertys>::Value>(std::ostream& refOstream) \
{}\
std::string Table::GetRedisFetchString()\
{\
	std::stringstream ssData;\
	ssData << "HMGET " << RedisTableName() << "_" << GetPrimaryKey();\
	GetRedisFetchString<0>(ssData);\
	return ssData.str();\
}\
template<int dwIndex> \
void Table::GetRedisFetchString(std::ostream& refOstream) \
{\
	typename DL::TypeAt<Propertys, dwIndex>::Result& refData = m_oPropertys; \
	if (refData.Data().Changed())\
	{\
		refOstream << " \"" << refData.Data().Name() << "\""; \
	}\
	GetRedisFetchString<dwIndex + 1>(refOstream); \
}\
template<> \
void Table::GetRedisFetchString<DL::Length<Table::Propertys>::Value>(std::ostream& refOstream) \
{}

#define PropertyTableName(Name) \
const char* RedisTableName(){ return #Name; }

class Table
{
public:
	PropertyTableName(table);
	PrimaryPropertyDefine(Table, int, RoleId);
	PropertyDefine(Table, double, TeamId);
	PropertyDefine(Table, FixString<64>, Name);

	typedef DERIDELIST(RoleId,
		TeamId, Name) Propertys;

	PropertyRdedisStringDeclare;

	//void OnRoleIdChange() {}
	void OnTeamIdChange()
	{
		int a = 0;
		++a;
	}

protected:
private:

	Propertys m_oPropertys;

	bool m_bChanged;
};

PropertyRdedisStringDefine;

#endif // !__Property_H__
