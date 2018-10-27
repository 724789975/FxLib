#ifndef __Property_H__
#define __Property_H__

#include "derive_list.h"

template<typename T, typename const char*(szName)()>
class Property
{
public:
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
	Type & Get##Name (){return m_oProperty.Value();}\
	void Set##Name (const Type& value){m_oProperty = value;}\
protected:\
private:\
	Property<Type, __##Name> m_oProperty;\
};

#define HasOnChangeDefine(C, F)\
template<typename T>\
struct C##Has##F;

#define HasOnChange(C, F)\
template<typename T>\
struct C##Has##F {\
	template<typename U, void (U::*)()> struct HELPS;\
	template<typename U> static char Check(HELPS<U, &U::##F>*);\
	template<typename U> static int Check(...);\
	const static bool Has = sizeof(Check<T>(0)) == sizeof(char);\
};

#define PropertyDefine(C, Type, Name) \
CommonPropertyDeclare(Type, Name)\
HasOnChange(C, On##Name##Change)\
template<bool>\
void Name##Changed(){}\
template<>\
void Name##Changed<true>();\
Type & Get##Name (){return m_oPropertys.Get##Name();}\
void Set##Name (const Type& value){return m_oPropertys.Set##Name(value);}\
void Set##Name (const Type& value, std::ostream& refOstream)\
{\
	refOstream << __FUNCTION__ << " old value : " << Get##Name();\
	m_oPropertys.Set##Name(value);\
	refOstream << ", new value : " << Get##Name();\
	Name##Changed<C##Has##On##Name##Change<C>::Has>(); \
}

	//Name##Changed<false>();\

#define PropertyChangeDefine(C, Name)\
template<>\
void C##::##Name##Changed<true>()\
{\
}

//TODO 需要继承的列表
class Table
{
public:
	PropertyDefine(Table, int, RoleId);
	PropertyDefine(Table, int, TeamId);


	typedef DERIDELIST_2(RoleId, TeamId) Propertys;

	//void OnRoleIdChange() {}
	void OnTeamIdChange() {}
protected:
private:

	Propertys m_oPropertys;

};

PropertyChangeDefine(Table, RoleId);
PropertyChangeDefine(Table, TeamId);

#endif // !__Property_H__
