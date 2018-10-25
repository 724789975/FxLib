#ifndef __Property_H__
#define __Property_H__

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

#define PropertyDefine(Type, Name) \
CommonPropertyDeclare(Type, Name)\
Type & Get##Name (){return m_oPropertys.Get##Name();}\
void Set##Name (const Type& value){return m_oPropertys.Set##Name(value);}\


#define PropertyList(...) \
class Propertys : __VA_ARGS__{};\

//TODO 需要继承的列表
class Table
{
public:
	CommonPropertyDeclare(int, RoleId);
	CommonPropertyDeclare(int, TeamId);

	//PropertyList(public RoleId,
	//	public TeamId
	//);

	template<class T> class Propertys;
	template<> class Propertys<NullType> {};

	template<typename T, typename U>
	class Propertys<TYPELIST_2(T, U)> {};
	//class Propertys : public TYPELIST_2(RoleId, TeamId) {};
	Propertys<TYPELIST_2(RoleId, TeamId)> m_oPropertys;
protected:
private:

};

#endif // !__Property_H__
