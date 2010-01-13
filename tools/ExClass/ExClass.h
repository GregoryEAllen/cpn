
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
        ExClass(const ExClass &other)
	       : id(ids++)	{
            printf("ExClass(%4d) constructed from ExClass(%4d)\n", id, other.id);
        }
		virtual ~ExClass() {
			printf("ExClass(%4d) destroyed\n", id);
		}
        ExClass &operator=(const ExClass &other) {
            printf("ExClass(%4d) copied over with %4d\n", id, other.id);
            id = other.id;
            return *this;
        }
		virtual void func1(void);
        virtual void constfunc(void) const;
	protected:
		static int ids;
		int id;
	};

#if USE_NAMESPACE > 0
}
#endif
