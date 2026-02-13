
#include "ExClass.h"

#if USE_NAMESPACE > 0
namespace Examples {
#endif
int ExClass::ids = 0;

void ExClass::func1(void) {
	printf("ExClass(%4d) func1 called\n", id);
}

void ExClass::constfunc(void) const {
	printf("ExClass(%4d) constfunc called\n", id);
}

#if USE_NAMESPACE > 0
}
#endif
