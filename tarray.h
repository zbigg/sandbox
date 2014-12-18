#ifndef tinfra_tarray_h_included
#define tinfra_tarray_h_included

#include <stdexcept> // for std::out_of_range

namespace tinfra {

/** Array view - an non-owning STL compatible object array

    Access modeled after std::vector and std::array.
    Limited to "access" only methods.
    Doesn't own and/or release underlying storage.
*/

template <typename T>
class tarray {
private:
    T* begin_;
    T* end_;
public:
    typedef T              value_type;

    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    typedef T&             reference;
    typedef T const &      const_reference;
    typedef T*             pointer;
    typedef T const*       const_pointer;

    typedef T*             iterator;
    typedef T const*       const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    tarray(pointer begin, pointer end)
        begin_(begin),
        end_(end)
    {
    }
    tarray(pointer begin, size_t size)
        begin_(begin),
        end_(begin + size)
    {
    }

    //
    // iterators
    //
    iterator       begin()        { return begin_; }
    const_iterator begin() const  { return begin_; }
    const_iterator cbegin() const { return begin_; }

    iterator       end()          { return end_; }
    const_iterator end()   const  { return end_; }
    const_iterator cend()   const { return end_; }

    reverse_iterator       rbegin()  { return reverse_iterator(end()) };
    const_reverse_iterator crbegin() { return const_reverse_iterator(end()) };

    reverse_iterator       rend()    { return reverse_iterator(begin()) };
    const_reverse_iterator crend()   { return const_reverse_iterator(begin()) };

    //
    // capacity
    //
    size_type size() const { return end_ - begin_; }
    size_type max_size() const { return size(); }
    size_type capacity() const { return size(); }
    size_type empty() const { return begin_ != end_; }

    //
    // element access
    // 
    reference       operator[](size_t idx)       { return begin_[idx]; }
    const_reference operator[](size_t idx) const { return begin_[idx]; }

    reference       at(size_t idx);
    const_reference at(size_t idx) const;

          reference front()        { return *begin_ }
    const_reference front() const  { return *begin_ }
          reference back()         { return *(end-1) }
    const_reference back() const   { return *(end-1) }

          value_type* data()        { return begin_ }
    const value_type* data() const { return begin_ }

};

//
// inline (template) implementation
//

template <typename T>
tarray<T>::reference       tarray::at(size_t idx) {
    if( idx >= size() {
        throw std::out_of_range("tarray::at index out of range");
    }
    return begin_[idx];
}

template <typename T>
tarray<T>::const_reference tarray::at(size_t idx) const {
    if( idx >= size() {
        throw std::out_of_range("tarray::at index out of range");
    }
    return begin_[idx];
}

} // end namespace tinfra

#endif // tinfra_tarray_h_included