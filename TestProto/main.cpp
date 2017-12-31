#include "test.pb.h"
#include "proto_dispatcher.h"
#include "callback_dispatch.h"

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

protected:
private:
	int a;
	int b;
};

int main(int argc, char **argv)
{
	AAA ta;
	CallBackDispatcher::ProtoDispatcher<BBB> aaa(std::string("asdf"));
	aaa.FuncReg<test>(std::bind(&(AAA::Test), &ta, std::placeholders::_1, std::placeholders::_2));

	CallBackDispatcher::ClassCallBackDispatcher<void, std::string, AAA, BBB, test> ta1(ta);
	ta1.RegistFunction("test", &(AAA::Fun));

	CallBackDispatcher::ClassCallBackDispatcher<void, std::string, AAA, BBB> ta2(ta);
	ta2.RegistFunction("test", &(AAA::Fun1));

	test ttt;
	ttt.set_id(123);
	ttt.set_str("asdf");
	return 0;
}
