
#pragma once
#include <iterator>

template<typename itr_t, typename diff_t, typename cat_t>
void circular_iterator_increment_impl(
        const itr_t &begin, const itr_t &end, itr_t &cur, diff_t n, cat_t)
{
    while (n--) {
        ++cur;
        if (cur == end) cur = begin;
    }
}
template<typename itr_t, typename diff_t>
void circular_iterator_increment_impl(
        const itr_t &begin, const itr_t &end, itr_t &cur, diff_t n, std::random_access_iterator_tag)
{
    diff_t left = end - cur;
    if (n >= left) {
        cur = begin;
        n -= left;
        left = end - cur;
        while (n > left) n -= left;
    }
    cur += n;
}

template<typename itr_t, typename diff_t, typename cat_t>
void circular_iterator_decrement_impl(
        const itr_t &begin, const itr_t &end, itr_t &cur, diff_t n, cat_t)
{
    while (n--) {
        if (cur == begin) cur = end;
        --cur;
    }
}

template<typename itr_t, typename diff_t>
void circular_iterator_decrement_impl(
        const itr_t &begin, const itr_t &end, itr_t &cur, diff_t n, std::random_access_iterator_tag)
{
    diff_t behind = cur - begin;
    if (n > behind) {
        cur = end;
        n -= behind;
        behind = cur - begin;
        while (n > behind) n -= behind;
    }
    cur -= n;
}

template<typename base_iterator>
class circular_iterator
: public std::iterator<typename std::iterator_traits<base_iterator>::iterator_category,
    typename std::iterator_traits<base_iterator>::value_type,
    typename std::iterator_traits<base_iterator>::difference_type,
    typename std::iterator_traits<base_iterator>::pointer,
    typename std::iterator_traits<base_iterator>::reference>
{
public:
    typedef base_iterator iterator_type;
    typedef typename std::iterator_traits<base_iterator>::iterator_category iterator_category;
    typedef typename std::iterator_traits<base_iterator>::difference_type difference_type;
    typedef typename std::iterator_traits<base_iterator>::reference reference;
    typedef typename std::iterator_traits<base_iterator>::pointer pointer;

    circular_iterator(iterator_type s, iterator_type e) : itr_start(s), current(s), itr_end(e) {}

    circular_iterator(iterator_type s, iterator_type c, iterator_type e) : itr_start(s), current(c), itr_end(e) {}

    circular_iterator(const circular_iterator<base_iterator> &o)
        : itr_start(o.begin()), current(o.base()), itr_end(o.end()) {}

    template<typename Itr>
    circular_iterator(const circular_iterator<Itr> &o)
        : itr_start(o.begin()), current(o.base()), itr_end(o.end()) {}

    iterator_type begin() const { return itr_start; }
    iterator_type base() const { return current; }
    iterator_type end() const { return itr_end; }

    reference operator*() const { return *current; }
    pointer operator->() const { return &*current; }

    circular_iterator operator++() {
        incr();
        return *this;
    }

    circular_iterator operator++(int) {
        circular_iterator tmp = *this;
        incr();
        return tmp;
    }

    circular_iterator operator--() {
        decr();
        return *this;
    }

    circular_iterator operator--(int) {
        circular_iterator tmp = *this;
        decr();
        return tmp;
    }

    circular_iterator operator+(difference_type n) const {
        circular_iterator tmp = *this;
        tmp.incr(n);
        return tmp;
    }

    circular_iterator &operator+=(difference_type n) {
        incr(n);
        return *this;
    }

    circular_iterator operator-(difference_type n) const {
        circular_iterator tmp = *this;
        tmp.decr(n);
        return tmp;
    }

    circular_iterator &operator-=(difference_type n) {
        decr(n);
        return *this;
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }

protected:
    void incr() {
        ++current;
        if (current == itr_end) {
            current = itr_start;
        }
    }

    void incr(difference_type n) {
        circular_iterator_increment_impl(itr_start, itr_end,
                current, n, iterator_category());
    }

    void decr() {
        if (current == itr_start) {
            current = itr_end;
        }
        --current;
    }

    void decr(difference_type n) {
        circular_iterator_decrement_impl(itr_start, itr_end,
                current, n, iterator_category());
    }

    base_iterator itr_start;
    base_iterator current;
    base_iterator itr_end;
};

template<typename ItrR, typename ItrL>
inline bool operator==(const circular_iterator<ItrR> &a,
        const circular_iterator<ItrL> &b) { return a.base() == b.base(); }

template<typename ItrR, typename ItrL>
inline bool operator!=(const circular_iterator<ItrR> &a,
        const circular_iterator<ItrL> &b) { return a.base() != b.base(); }

