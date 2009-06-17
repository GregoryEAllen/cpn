
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
		void func1(void);
	private:
		static int ids;
		int id;
	};

#if USE_NAMESPACE > 0
}
#endif
