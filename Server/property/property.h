#ifndef __Property_H__
#define __Property_H__

template<typename T>
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

	const T& Value()const
	{
		return m_tValue;
	}

private:
	bool m_bChanged;
	T m_tValue;
};

#endif // !__Property_H__
