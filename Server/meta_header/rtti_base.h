#ifndef __CRTTI_BASE_H__
#define __CRTTI_BASE_H__

//运行时类型 只要通过实现接口的方式实现
//将所有类型都放到union中 所有类型都是pod的
//并且通过仿函数继承实现虚函数功能

#include <new>
#include <string.h>

namespace RTTI
{
	struct CRTTIBase
	{
		const char* (*GetClassName)();
		int (*GetType)();
	};

	template<typename T>
	struct CRTTIInterface : public CRTTIBase
	{
		typedef T Interface;
	};

};

#define INTERFACE_DECLARE_OPERATORS(InterFaceName) \
	typedef RTTI::CRTTIInterface<InterFaceName> Parent;\
	typedef Parent::Interface Interface;\
	enum { depth = 0, };\
	\
	void Construct(const char* (*p_szNameFunc)(), int(*p_dwGetTypeFunc)())\
	{\
		RTTI::CRTTIBase::GetClassName = p_szNameFunc;\
		RTTI::CRTTIBase::GetType = p_dwGetTypeFunc;\
	}\

#define INSTANCE_DECLARE_OPERATORS(ParentName, InstanceName, Type) \
	typedef ParentName Parent;\
	typedef Parent::Interface Interface;\
	enum { depth = Parent::depth + 1, };\
	static const char* GetClassName() { return #InstanceName ; }\
	static int GetType() { return Type; }\


struct Test1 : public RTTI::CRTTIInterface<Test1>
{
	INTERFACE_DECLARE_OPERATORS(Test1);

	class TestFun
	{
	public:
		virtual int operator()() { return 0; };
		int a;
	};

	TestFun* tt1;
};

struct Test2 : public Test1
{
	INSTANCE_DECLARE_OPERATORS(Test1, Test2, 2);

	class TestFun : public Interface::TestFun
	{
	public:
		virtual int operator()() { return 0; }
	};

	void Construct()
	{
		Interface::Construct(GetClassName, GetType);
	}

private:
	int b;
	TestFun t2;
};

struct Test3 : public Test1
{
	INSTANCE_DECLARE_OPERATORS(Test1, Test3, 3);
	
	void Construct()
	{
		Interface::Construct(GetClassName, GetType);
	}

};

struct Test4 : public Test3
{
	INSTANCE_DECLARE_OPERATORS(Test3, Test4, 4);

	class TestFun : public Interface::TestFun
	{
	public:
		TestFun(Test4& ref) : refTest4(ref) {}
		virtual int operator()()
		{
			refTest4.d = 30;
			return refTest4.d;
		}
		int b;

		Test4& refTest4;
	};

	void Construct()
	{
		Interface::Construct(GetClassName, GetType);

		//处理继承的方法
		new(&t2) TestFun(*this);
		Interface::tt1 = &t2;
	}

	int d;

	TestFun t2;
};

union Content
{
	Content() {}
	Test1 t1;
	Test2 t2;
	Test3 t3;
	Test4 t4;
};







#endif // !__CRTTI_BASE_H__
