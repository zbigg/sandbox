#include <cstddef>
#include <cassert>
#include <new>
#include <type_traits>
#include <atomic>
#include <inttypes.h>
#include <stdio.h>

#include <thread>
#include <vector>
#include <iostream>

#include "tinfra/fmt.h"
#include "tinfra/stream.h"

template <typename T>
class atomic_segregated_memory_allocator {
    static_assert(sizeof(T) >= sizeof(int), "T shall be able to store int basic_linked_memory_pool ");

    T* const        buffer_start;
    const size_t    buffer_size;
    std::atomic<int> next_free;
    std::atomic<int> n_allocated;

public:
    atomic_segregated_memory_allocator(T* ptr, size_t size):
        buffer_start(ptr),
        buffer_size(size),
        next_free(0),
        n_allocated(0)
    {
        this->initialize_storage();
    }

    // C++-like allocator interface
    T* allocate() {
        int allocated_item = next_free.load();

        while( true ) { 
            if( allocated_item == -1 ) {
                throw std::bad_alloc();
            }
            int new_next_free = indicator(allocated_item);
            assert(new_next_free != allocated_item);
            const bool change_succeded = next_free.compare_exchange_strong(allocated_item, new_next_free);
            if( !change_succeded ) {
                tprintf(tinfra::err, "allocate, retrying transaction\n");
                continue;
            }
            break;
        }
        ++n_allocated;
        return item_ptr(allocated_item);
    }
    void deallocate(T* p) {
        assert( p >= this->buffer_start && p < this->buffer_start + this->buffer_size);
        //assert( p % sizeof(T) == 0 );

        const int idx = (p - this->buffer_start);
        int       last_next_free = next_free.load();
        while( true ) {
            assert(last_next_free != idx);
            this->indicator(idx) = last_next_free;
                // this assertion sometimes fails, so we have some inconsistency
                // in free slot graph !!!
            const bool change_succeded = next_free.compare_exchange_strong(last_next_free, idx);
            if( !change_succeded ) {
                tprintf(tinfra::err, "deallocate, retrying transaction\n");
                continue;
            }
            break;
        }
        --n_allocated;
    }

    int allocated() const {
        int r = this->buffer_size;
        int i = this->next_free;
        while( i != -1 ) {
            i = indicator(i);
            r -= 1;
        }
        return r;
        //return this->n_allocated.load(std::memory_order_relaxed);
    }
private:
    void initialize_storage()
    {
        for( size_t i = 0; i < this->buffer_size; i++ ) {
            indicator(i) = i+1;
        }
        indicator(this->buffer_size-1) = -1;
        this->next_free = 0;
    }
    const int& indicator(int i) const {
        return * reinterpret_cast<int const*>(this->buffer_start + i);
    }
    int& indicator(int i) {
        return * reinterpret_cast<int*>(this->buffer_start + i);
    }
    T*   item_ptr(int i) {
        return this->buffer_start + i;
    }

};

template <typename T, size_t N>
struct static_atomic_segregated_memory_pool {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
    atomic_segregated_memory_allocator<T> allocator;

public:
    static_atomic_segregated_memory_pool():
        allocator(reinterpret_cast<T*>(buffer), N)
    { }

    // C++ allocator interface
    T* allocate() {
        return this->allocator.allocate();
    }

    void deallocate(T* p) {
        this->allocator.deallocate(p);
    }
    int allocated() const {
        return this->allocator.allocated();
    }
};

#define PPTR(name) ::printf("%s = %p\n", #name, name)

static_atomic_segregated_memory_pool<int, 1024> pool;

void test0()
{
    assert( pool.allocated() == 0);
    int* p = pool.allocate();
    assert( pool.allocated() == 1);
    pool.deallocate(p);
    assert( pool.allocated() == 0);
}

void test1()
{
    const int R = 1024;
    int* ptrs[R];
    assert( pool.allocated() == 0);

    for( int r = 0; r < R; r++ ) {
        ptrs[r] = pool.allocate();
        assert( pool.allocated() == r+1);
    }

    for( int r = 0; r < R; r++ ) {
        pool.deallocate(ptrs[r]);
        assert( pool.allocated() == 1024-(r+1));
    }

    assert( pool.allocated() == 0);
}


void f(int T)
{
    const int R = 1024;
    const int N = 10;
    int* ptr[N];
    for( int r = 0; r < R; r++ ) {
        //tinfra::tprintf(tinfra::err, "T[%i] before iter %i pool size %i\n", T, r, pool.allocated());
        for( int i = 0; i < N; ++i ) {
            ptr[i] = pool.allocate();
            //tinfra::tprintf(tinfra::err, "T[%i] allocated %i\n", T, ptr[i]);
        }

        for( int i = 0; i < N; ++i ) {
            //tinfra::tprintf(tinfra::err, "T[%i] deallocating %i\n", T, ptr[i]);
            pool.deallocate(ptr[i]);
        }
        //tinfra::tprintf(tinfra::err, "T[%i] before iter %i pool size %i\n", T, r, pool.allocated());
    }
}


void test3() 
{
    assert( pool.allocated() == 0);
    std::vector<std::thread> v;
    for (int n = 0; n < 10; ++n) {
        v.emplace_back(f, n);
    }
    for (auto& t : v) {
        t.join();
    }
    tinfra::tprintf(tinfra::err, "T[main] after test pool size %i\n",pool.allocated());
}

int main()
{
    test0();
    test1();
    test3();
}