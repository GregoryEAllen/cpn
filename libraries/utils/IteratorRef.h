
/** \file
 * \author John Bridgman
 * \brief This allows the encapsulation of an arbitrary iterator
 * of a specific type. This allows for non template functions to
 * take an iterator from any stl container that has the given type.
 *
 * Note that this iterator does not implement the full iterator
 * interface. This is also less efficient.
 */
#ifndef ITERATORREF_H
#define ITERATORREF_H
#pragma once
#include <iterator>
#include <typeinfo>
#include <memory>
/**
 * \brief A reference to an iterator.
 *
 * This iterator container class lets a non templated function take
 * a generic iterator from several different types of objects.
 * This way a function can take a beginning and ending iterator
 * from a deque or a list or a vector or a actual pointer and without
 * having to be a template.
 */
template<typename T>
class IteratorRef {
private:

    class ItrRef {
    public:
        virtual ~ItrRef() {}
        virtual void Increment() = 0;
        virtual void Decrement() = 0;
        virtual T &Dereference() = 0;
        virtual ItrRef *Clone() const = 0;
        virtual bool Equals(const ItrRef *rhs) const = 0;

    };

    template<typename iterator_type>
    class ItrRefImpl : public ItrRef {
    public:
        typedef typename std::iterator_traits<iterator_type>::value_type value_type;
        ItrRefImpl(iterator_type itr_) : itr(itr_) {}
        void Increment() { ++itr; }
        void Decrement() { --itr; }
        value_type &Dereference() { return *itr; }
        ItrRef *Clone() const { return new ItrRefImpl<iterator_type>(itr); }
        bool Equals(const ItrRef *rhs) const {
            try {
                const ItrRefImpl<iterator_type> *o = dynamic_cast<const ItrRefImpl<iterator_type> *>(rhs);
                if (o) {
                    return itr == o->itr;
                }
            } catch (const std::bad_cast&) {}
            return false;
        }
    private:
        iterator_type itr;
    };

public:

    template<typename iter_type>
    IteratorRef(iter_type itr)
    : itrref(std::auto_ptr<ItrRef>(new ItrRefImpl<iter_type>(itr)))
    {}

    IteratorRef(const IteratorRef &itr)
        :itrref(itr.itrref->Clone())
    {}

    IteratorRef &operator=(const IteratorRef &itr) {
        itrref.reset(itr.itrref->Clone());
        return *this;
    }

    T &operator*() {
        return itrref->Dereference();
    }

    T *operator->() {
        return &itrref->Dereference();
    }

    IteratorRef operator++(int) {
        IteratorRef copy = *this;
        itrref->Increment();
        return copy;
    }

    IteratorRef operator++() {
        itrref->Increment();
        return *this;
    }

    IteratorRef operator--(int) {
        IteratorRef copy = *this;
        itrref->Decrement();
        return copy;
    }

    IteratorRef operator--() {
        itrref->Decrement();
        return *this;
    }

    bool operator==(const IteratorRef<T> &rhs) const {
        return itrref->Equals(rhs.itrref.get());
    }

    bool operator!=(const IteratorRef<T> &rhs) const {
        return !itrref->Equals(rhs.itrref.get());
    }

private:
    std::auto_ptr<ItrRef> itrref;
};


#endif
