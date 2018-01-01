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
	bool Fun2(BBB& a, google::protobuf::Message& t1) { return false; }
protected:
private:
	int a;
	int b;
};

int main(int argc, char **argv)
{
	AAA ta;
	CallBackDispatcher::ProtoCallBackDispatch<AAA, BBB> ta3(ta);
	//ta3.GetFunction(NULL);
	typedef bool (AAA::*F)(BBB& a, google::protobuf::Message& t1);

	//F f;
	//f = AAA::Fun2;

	bool (AAA::*pf)(BBB& a, google::protobuf::Message& t1) = &AAA::Fun2;
	ta3.RegistFunction(test::descriptor(), &AAA::Fun2);

	BBB b;
	ta3.Dispatch("test", NULL, 0, NULL, b);

	test ttt;
	ttt.set_id(123);
	ttt.set_str("asdf");
	return 0;
}
