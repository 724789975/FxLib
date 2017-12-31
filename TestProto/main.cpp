#include "test.pb.h"
#include "proto_dispatcher.h"

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

protected:
private:
	int a;
	int b;
};

int main(int argc, char **argv)
{
	AAA ta;
	ProtoBufDispatch::ProtoDispatcher<BBB> aaa(std::string("asdf"));
	aaa.FuncReg<test>(std::bind(&(AAA::Test), &ta, std::placeholders::_1, std::placeholders::_2));

	test ttt;
	ttt.set_id(123);
	ttt.set_str("asdf");
	return 0;
}
