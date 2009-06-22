
#include <cstdio>
#define USE_NAMESPACE 0
#if USE_NAMESPACE > 0
namespace Examples {
#endif


	class ExClass {
	public:
		ExClass()
	       : id(ids++)	{
			printf("ExClass(%4d) constructed\n", id);
		}
		~ExClass() {
			printf("ExClass(%4d) destroyed\n", id);
		}
		virtual void func1(void);
	protected:
		static int ids;
		int id;
	};

#if USE_NAMESPACE > 0
}
#endif
