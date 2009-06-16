/** \file
 * Template class for a shared pointer. For refcnterences see the
 * many hundreds of descriptions, examples, heated discussions at
 * http://www.google.com/search?q=shared+pointer
 */

#include <new>

/**
 * \warning NOT THREAD SAFE
 */
template<class T>
class SharedPtr {
public:
	typedef unsigned long ulong;

	SharedPtr() throw() p(0), refcnt(0) {}

	SharedPtr(T* pointer) throw(bad_alloc) : p(pointer), refcnt(0) {
		try {
			refcnt = new ulong;
			*refcnt = 1;
		} catch (bad_alloc &e) {
			delete p;
			p = 0;
			throw e;
		}
	}

	SharedPtr(SharedPtr<T> &a) throw() {
		Aquire(a);
	}

	template<class H>
	SharedPtr(SharedPtr<H> &a) throw() {
		Aquire(a);
	}

	~SharedPtr() {
		Release();
	}

	SharedPtr<T> &operator=(SharedPtr<T> &a) throw() {
		Release();
		Aquire(a);
	}

	template<class H>
	SharedPoitner<T> &operator=(SharedPtr<H> &a) throw() {
		Release();
		Aquire(a);
	}

	const T &operator*() const throw() { return &p; }

	const T &operator->() const throw() { return &p; }

	void Release(void) throw() {
		if (p) {
			if (*refcnt > 1) {
				(*refcnt)--;
				refcnt = 0;
				p = 0;
			} else {
				delete refcnt;
				refcnt = 0;
				delete p;
				p = 0;
			}
		}
	}

private:

	void Aquire(SharedPtr<T> &a) {
		p = a.p;
		refcnt = a.refcnt;
		(*refcnt)++;
	}

	template<class H>
	void Aquire(SharedPtr<H> &a) {
		p = a.p;
		refcnt = a.refcnt;
		(*refcnt)++;
	}

	T *p;
	ulong *refcnt;
};
