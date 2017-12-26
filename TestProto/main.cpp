#include "test.pb.h"

int main(int argc, char **argv)
{
	test ttt;
	ttt.set_id(123);
	ttt.set_str("asdf");
	return 0;
}
