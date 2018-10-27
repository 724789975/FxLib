#ifndef __Property_H__
#define __Property_H__

#include "derive_list.h"

template<typename T, const char*(szName)()>
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

#define PropertyDeclare(C, Type, Name) \
CommonPropertyDeclare(Type, Name)\
HasOnChange(C, On##Name##Change)\
template<bool> \
void Name##Changed();\
Type & Get##Name ();\
void Set##Name (const Type& value);\
void Set##Name (const Type& value, std::ostream& refOstream);

#define PropertyDefine(C, Type, Name) \
template<bool> \
void C::Name##Changed(){}\
template<> \
void C::Name##Changed<true>(){}\
Type & C::Get##Name (){return m_oPropertys.Get##Name();}\
void C::Set##Name (const Type& value){return m_oPropertys.Set##Name(value);}\
void C::Set##Name (const Type& value, std::ostream& refOstream)\
{\
	refOstream << __FUNCTION__ << " old value : " << Get##Name();\
	m_oPropertys.Set##Name(value);\
	refOstream << ", new value : " << Get##Name();\
	Name##Changed<C##Has##On##Name##Change<C>::Has>(); \
}

//TODO 需要继承的列表
class Table
{
public:
	PropertyDeclare(Table, int, RoleId);
	PropertyDeclare(Table, int, TeamId);

	typedef DERIDELIST_2(RoleId, TeamId) Propertys;

	//void OnRoleIdChange() {}
	void OnTeamIdChange() {}
protected:
private:

	Propertys m_oPropertys;

};

PropertyDefine(Table, int, RoleId);
PropertyDefine(Table, int, TeamId);


#endif // !__Property_H__
