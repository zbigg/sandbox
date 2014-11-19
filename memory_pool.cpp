#include <cstddef>
#include <cassert>
#include <new>
#include <type_traits>
#include <inttypes.h>
#include <stdio.h>

template <typename T>
class basic_linked_list_allocator {
    static_assert(sizeof(T) > sizeof(int), "T shall be able to store int basic_linked_memory_pool ");

    T* const     buffer_start;
    const size_t buffer_size;
    int          next_free;
    int          allocated;

public:
    basic_linked_list_allocator(T* ptr, size_t size):
        buffer_start(ptr),
        buffer_size(size)
    {
        this->initialize_storage();
    }

    // C++ allocator interface
    T* allocate(size_t n) {
        assert(n == 1);
        if( next_free == -1 ) {
            throw std::bad_alloc();
        }
        const int current = next_free;
        next_free = indicator(next_free);
        return item_ptr(current);
    }
    void deallocate(T* p, size_t n = 1) {
        assert( n == 1 );
        assert( p >= this->buffer_start && p < this->buffer_start + this->buffer_size);
        //assert( p % sizeof(T) == 0 );

        const int idx = (p - this->buffer_start);
        this->indicator(idx) = next_free;
        this->next_free = idx;
    }

private:
    void initialize_storage()
    {
        for( int i = 0; i < this->buffer_size; i++ ) {
            indicator(i) = i+1;
        }
        indicator(this->buffer_size-1) = -1;
        this->next_free = 0;
    }
    int& indicator(int i) {
        return * reinterpret_cast<int*>(this->buffer_start + i);
    }
    T*   item_ptr(int i) {
        return this->buffer_start + i;
    }

};

template <typename T, size_t N>
struct basic_linked_list_pool {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
    basic_linked_list_allocator<T> allocator;

public:
    basic_linked_list_pool():
        allocator(reinterpret_cast<T*>(buffer), N)
    { }

    // C++ allocator interface
    T* allocate(size_t n) {
        return this->allocator.allocate(n);
    }

    void deallocate(T* p, size_t n = 1) {
        this->allocator.deallocate(p, n);
    }
};


template <typename TAGTYPE, typename T, size_t N> 
struct linked_allocator {
    typedef basic_linked_list_pool<T, N> storage_type;
    static storage_type storage;

    T* allocate(size_t n, void* hint = nullptr) {
        return storage.allocate(n);
    }
    void deallocate(T* p) {
        return storage.deallocate(p);
    }

};

template <typename TAGTYPE, typename T, size_t N>
basic_linked_list_pool<T, N> linked_allocator<TAGTYPE,T,N>::storage;

struct transport_block {
    const char* buf;
    uint16_t      rnti;
    uint8_t       mcs;
    uint8_t       rb;
};

struct foo_tag;
typedef linked_allocator<foo_tag, transport_block, 1024> foo_transport_block_allocator_t;

#define PPTR(name) ::printf("%s = %p\n", #name, name)

int main()
{
    //foo_transport_block_allocator::initialize_storage();

    foo_transport_block_allocator_t p;
    transport_block* p1 = p.allocate(1);
    p1->buf = "foo";
    p1->rnti = 16;
    p1->mcs  = 32;
    p1->rb   = 2;
    PPTR(p1);
    transport_block* p2 = p.allocate(1);
    p2->buf = "baz";
    p2->rnti = 17;
    p2->mcs  = 33;
    p2->rb   = 3;
    PPTR(p2);
    p.deallocate(p1);
    transport_block* p3 = p.allocate(1);
    PPTR(p3);
    p3->buf = "bar";
    p3->rnti = 18;
    p3->mcs  = 34;
    p3->rb   = 4;
}