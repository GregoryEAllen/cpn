

#include "ExClass.h"

class ExClass2 : public ExClass {
public:
	void func1(void) {
		printf("ExClass2(%4d) func1 called\n", id);
	}
};

int main(int argc, char **argv) {
	ExClass blah;
	ExClass2 nyu;
	nyu.func1();
	blah = nyu;
	blah.func1();
	return 0;
}
