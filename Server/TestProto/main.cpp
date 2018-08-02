#include "test.pb.h"
#include "proto_dispatcher.h"
#include "callback_dispatch.h"

#include <iostream>
#include "fxtimer.h"
#include "fxmeta.h"

class BBB
{
public:
	BBB(){}
};

class AAA
{
public:
	AAA(){}

	void Test(BBB* a, const test& t1){}
	void Fun(BBB& a, test& t1){}
	void Fun1(BBB& a){}
	bool Fun2(BBB& a, google::protobuf::Message& t1) { return false; }
protected:
private:
	int a;
	int b;
};

class Test : public CEventCaller<Test, 9>
{
public:
	Test()
	{
	}

	~Test()
	{
	}

	void F1()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddSecontTimer(12, &CEventCaller<Test, 1>::GetEvent());
	}
	void F2()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddSecontTimer(15, &CEventCaller<Test, 2>::GetEvent());
	}
	void F3()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddDayHourTimer(5, &CEventCaller<Test, 3>::GetEvent());
	}
	void F4()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddDayHourTimer(4, &CEventCaller<Test, 4>::GetEvent());
	}
	void F5()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddHourTimer(2, &CEventCaller<Test, 5>::GetEvent());
	}
	void F6()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddMinuteTimer(15, &CEventCaller<Test, 6>::GetEvent());
	}
	void F7()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddMinuteTimer(12, &CEventCaller<Test, 7>::GetEvent());
	}
	void F8()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddTimer(15, &CEventCaller<Test, 8>::GetEvent());
	}
	void F9()
	{
		LogExe(LogLv_Debug3, "%s", "");
		GetTimeHandler()->AddTimer(10, &CEventCaller<Test, 9>::GetEvent());
	}

private:

};




int main(int argc, char **argv)
{
	AAA ta;
	CallBackDispatcher::ProtoCallBackDispatch<AAA, BBB> ta3(ta);
	//ta3.GetFunction(NULL);
	ta3.RegistFunction(test::descriptor(), &AAA::Fun2);

	BBB b;
	ta3.Dispatch("test", NULL, 0, NULL, b);

	test ttt;
	ttt.set_id(123);
	ttt.set_str("asdf");

	GetTimeHandler()->Init();
	GetTimeHandler()->Run();


	Test t1;
	GetTimeHandler()->AddTimer(10, &t1.CEventCaller<Test, 9>::MakeEvent(&t1, &Test::F9));
	GetTimeHandler()->AddTimer(9, &t1.CEventCaller<Test, 8>::MakeEvent(&t1, &Test::F8));
	GetTimeHandler()->AddTimer(8, &t1.CEventCaller<Test, 7>::MakeEvent(&t1, &Test::F7));
	GetTimeHandler()->AddTimer(7, &t1.CEventCaller<Test, 6>::MakeEvent(&t1, &Test::F6));
	GetTimeHandler()->AddTimer(6, &t1.CEventCaller<Test, 5>::MakeEvent(&t1, &Test::F5));
	GetTimeHandler()->AddTimer(5, &t1.CEventCaller<Test, 4>::MakeEvent(&t1, &Test::F4));
	GetTimeHandler()->AddTimer(4, &t1.CEventCaller<Test, 3>::MakeEvent(&t1, &Test::F3));
	GetTimeHandler()->AddTimer(3, &t1.CEventCaller<Test, 2>::MakeEvent(&t1, &Test::F2));
	GetTimeHandler()->AddTimer(2, &t1.CEventCaller<Test, 1>::MakeEvent(&t1, &Test::F1));
	while (true)
	{
		GetTimeHandler()->Run();
		FxSleep(1);
	}
	return 0;
}
