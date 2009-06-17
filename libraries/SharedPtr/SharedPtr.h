/** \file
 * Template class for a shared pointer. For refcnterences see the
 * many hundreds of descriptions, examples, heated discussions at
 * http://www.google.com/search?q=shared+pointer
 */

#include <new>
template<class T> class SharedPtr;

/**
 * a releases it's contents if it has any and aquires
 * the contents of b. If a == b then this causes 
 * the contents to be removed.
 */
/*
 *
template <class H, class J>
void SharedPtrAquire(SharedPtr<H> &a, SharedPtr<J> &b) {
	a.Release();
	if (b.p) {
		a.p = b.p;
		a.refcnt = b.refcnt;
		(*a.refcnt)++;
	}
}
*/

/**
 * \warning NOT THREAD SAFE
 */
template<class T>
class SharedPtr {
public:
	typedef unsigned long ulong;

	//template <class H, class J> friend void SharedPtrAquire(SharedPtr<H> a, SharedPtr<J> b);

	SharedPtr() throw() {
		p = 0;
		refcnt = 0;
	}

	explicit SharedPtr(T* pointer) throw(std::bad_alloc) {
		try {
			p = pointer;
			refcnt = new ulong;
			*refcnt = 1;
		} catch (std::bad_alloc &e) {
			delete p;
			p = 0;
			throw;
		}
	}

	SharedPtr(const SharedPtr<T> &a) throw() {
		p = 0;
		refcnt = 0;
		//SharedPtrAquire(*this, a);
		Aquire(a);
	}

	template<class H>
	SharedPtr(const SharedPtr<H> &a) throw() {
		p = 0;
		refcnt = 0;
		//SharedPtrAquire(*this, a);
		Aquire(a);
	}

	~SharedPtr() {
		Release();
	}

	SharedPtr<T> &operator=(const SharedPtr<T> &a) throw() {
		if (this != &a) {
			//SharedPtrAquire(*this, a);
			Aquire(a);
		}
		return *this;
	}

	template<class H>
	SharedPtr<T> &operator=(const SharedPtr<H> &a) throw() {
		//SharedPtrAquire(*this, a);
		Aquire(a);
		return *this;
	}

	T& operator*() const throw() { return *p; }

	T* operator->() const throw() { return p; }

	T* Get() const throw() { return p; }

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

	void Aquire(const SharedPtr<T> &a) {
		Release();
		if (a.p) {
			p = a.p;
			refcnt = a.refcnt;
			(*refcnt)++;
		} else {
			p = 0;
			refcnt = 0;
		}
	}

	template<class H> friend class SharedPtr;

	template<class H>
	void Aquire(const SharedPtr<H> &a) {
		Release();
		if (a.p) {
			p = a.p;
			refcnt = a.refcnt;
			(*refcnt)++;
		} else {
			p = 0;
			refcnt = 0;
		}
	}

	T *p;
	ulong *refcnt;

};

