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

	void Test(BBB a, test t1){}
protected:
private:
	int a;
	int b;
};

int main(int argc, char **argv)
{
	AAA ta;
	ProtoBufDispatch::ProtoDispatcher<BBB> aaa(std::string("asdf"));
	ProtoBufDispatch::ProtoDispatcher<ProtoBufDispatch::NullType> a1aa("asd1f");
	//aaa.FuncReg<test>(std::bind(&AAA::Test, std::placeholders::_1, std::placeholders::_2));

	test ttt;
	ttt.set_id(123);
	ttt.set_str("asdf");
	return 0;
}
