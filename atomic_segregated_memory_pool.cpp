#include <cstddef>
#include <cassert>
#include <new>
#include <type_traits>
#include <atomic>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include <thread>
#include <vector>
#include <map>
#include <iostream>

#include "tinfra/fmt.h"
#include "tinfra/stream.h"


int thread_count = 0;
thread_local int thread_log_idx;

std::string get_thread_string()
{
    std::string foo(thread_count, '.');
    foo[thread_log_idx] = '|';
    return tinfra::tsprintf("%i-%s", std::this_thread::get_id(), foo);
}

template <typename T>
std::string vector_to_string(T const& v)
{
    std::ostringstream foo;
    foo << "[";
    for( size_t i = 0; i < v.size(); ++i ) {
        if( i != 0 ) {
            foo << ", ";
        }
        foo << v[i];
    }
    foo << "]";
    return foo.str();
}

#define USE_STD_ATOMIC
#ifdef USE_STD_ATOMIC

#define ATOMMOO std::atomic

#else
#include <mutex>


template <typename T>
class my_mutex_based_fake_atomic {
    T protected_var;
    mutable std::mutex mux;
public:
    my_mutex_based_fake_atomic():
        protected_var()
    {
    }
    
    my_mutex_based_fake_atomic(T v):
        protected_var(v)
    {
    }

    my_mutex_based_fake_atomic<T>& operator=(T v) {
        std::lock_guard<std::mutex> g(this->mux);
        protected_var = v;
        return *this;
    }

    my_mutex_based_fake_atomic<T>& operator++() { // prefix ++
        std::lock_guard<std::mutex> g(this->mux);
        protected_var++;
        return *this;
    }
    
    my_mutex_based_fake_atomic<T>& operator--() { // prefix ++
        std::lock_guard<std::mutex> g(this->mux);
        protected_var--;
        return *this;
    }
    /*
    my_mutex_based_fake_atomic<T> operator++(int) {
        std::lock_guard<std::mutex> g(this->mux);
        my_mutex_based_fake_atomic<T> result(protected_var);
        protected_var++;
        return *this;
    }
    */

    T load() const {
        std::lock_guard<std::mutex> g(this->mux);
        return protected_var;
    }

    bool compare_exchange_strong(T& expected, T desired) {
        std::lock_guard<std::mutex> g(this->mux);
        if( protected_var == expected ) {
            protected_var = desired;
            tprintf(tinfra::err, "T[%i]: compare_exchange_strong commited %i (expected=%i)\n", get_thread_string(), desired, expected);
            return true;
        } else {
            tprintf(tinfra::err, "T[%i]: compare_exchange_strong not commited %i (expected=%i, actual=%i)\n", get_thread_string(), desired, expected, protected_var);
            expected = protected_var;
            return false;
        }
    }
};

#define ATOMMOO my_mutex_based_fake_atomic

#endif

static void my_nanosleep(int n)
{
    struct timespec f;
    f.tv_sec = 0;
    f.tv_nsec = n;
    ::nanosleep(&f, 0);
}

#define my_assert(foo) \
    do { \
        if( !(foo) ) { \
            tprintf(tinfra::err, "T[%i]: assertion '%s' failed in %s:%i\n", get_thread_string(), #foo, __FILE__, __LINE__); \
            abort(); \
        } \
    } while(0)
        
template <typename T>
class atomic_segregated_memory_allocator {
    static_assert(sizeof(T) >= sizeof(int), "T shall be able to store int basic_linked_memory_pool ");

    T* const        buffer_start;
    const size_t    buffer_size;
    ATOMMOO<unsigned long> next_free;
    ATOMMOO<int>  txn_id;
    ATOMMOO<int> n_allocated;

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
        unsigned long allocated_item_coded = next_free.load();
        
        int allocated_item;
        //tprintf(tinfra::err, "T[%i]: allocate first try with allocated_item %i\n", get_thread_string(), idx_decode(allocated_item_coded));
        while( true ) {
            allocated_item = idx_decode(allocated_item_coded);
            if( allocated_item == -1 ) {
                throw std::bad_alloc();
            }
            const int new_next_free = item_next_free_idx(allocated_item);
                // so here is the race
                //  1  (allocated_item AI, new_next_free NNF) pair is "read"
                //  2. (tries to commit, but ... in meantime)
                //  3.   (other thread allocates AI as a race)
                //  3.1  (NNF is allocates by other thread) ...
                //  4.   (other thread deallocates AI and it appears again as NF but now points to NNF2) 
                //  5. (commit succedds with NNF because AI is same as in 1) 
                //     BUG, NF points at NNF, which is not necessarily allocated
                //     should point at NNF2 (!?) impossible
                //     or TXN this kind of race should be detected and whole TXN shall be retried
            assert(new_next_free != allocated_item);
            //if(rand() % 2 ) my_nanosleep(1);
            const unsigned long new_next_free_coded = idx_encode(new_next_free);
            const bool change_succeded = next_free.compare_exchange_strong(allocated_item_coded, new_next_free_coded);
            if( !change_succeded ) {
                allocated_item = idx_decode(allocated_item_coded);
                //tprintf(tinfra::err, "T[%i]: allocate, retrying with allocated_item %i\n", get_thread_string(), allocated_item);
                continue;
            } else {
                //tprintf(tinfra::err, "T[%i]: allocated %i, next_free = %i\n", get_thread_string(), allocated_item, new_next_free);
            }
            break;
        }
        my_assert(_check_allocated(allocated_item));
        ++n_allocated;
        return item_ptr(allocated_item);
    }
    void deallocate(T* p) {
        assert( p >= this->buffer_start && p < this->buffer_start + this->buffer_size);
        //assert( p % sizeof(T) == 0 );

        const int idx = (p - this->buffer_start);
        unsigned long last_next_free_coded = next_free.load();
        int        last_next_free = idx_decode(last_next_free_coded);

        //tprintf(tinfra::err, "T[%i]: deallocate[%s], first try with last_next_free %i\n", get_thread_string(), idx, idx_decode(last_next_free_coded));
        my_assert( _check_allocated(idx) );

        while( true ) {
            
            my_assert(last_next_free != idx);
            //if(rand() % 2 ) my_nanosleep(1);
            this->item_next_free_idx(idx) = last_next_free;
                // this assertion sometimes fails, so we have some inconsistency
                // in free slot graph !!!
            const unsigned long idx_coded = idx_encode(idx);
            const bool change_succeded = next_free.compare_exchange_strong(last_next_free_coded, idx_coded);
            if( !change_succeded ) {
                last_next_free = idx_decode(last_next_free_coded);
                //tprintf(tinfra::err, "T[%i]: deallocate[%s], retrying with last_next_free %i\n", get_thread_string(), idx, last_next_free);
                continue;
            } else {
                //tprintf(tinfra::err, "T[%i]: deallocate[%i], next_free = %i (linked %i->%i)\n", get_thread_string(), idx, idx, idx, last_next_free);
            }
            break;
        }
        --n_allocated;
    }

    int allocated() const {
        int r = this->buffer_size;
        int i = this->next_free.load();
        while( i != -1 ) {
            i = item_next_free_idx(i);
            r -= 1;
        }
        return r;
        //return this->n_allocated.load(std::memory_order_relaxed);
    }
private:
    unsigned long idx_encode(int x)
    {
        const long new_txn_id = ++txn_id;
        return x | ( (unsigned long)new_txn_id << 32 );
    }
    int idx_decode(unsigned long x)
    {
        return int( x & 0xffffffff );
    }

    bool _check_for_loops() {
        std::vector<int>           visited_stream;
        std::map<int, size_t> when_visited; // (v at k) say, that k is already visited at position v
        int idx = idx_decode(next_free.load());
        int last = -1;
        while( idx != -1 ) {
            if( when_visited.find(idx) != when_visited.end() ) {
                // we've already seen this value, so WE HAVE LOOP

                tprintf(tinfra::err, "T[%i]: _check_for_loops %i->%i is a loop, whole vector\n", get_thread_string(), last, idx, vector_to_string(visited_stream));
                return false;
            }

            when_visited[idx] = visited_stream.size();
            visited_stream.push_back(idx);
            last = idx;
            idx = item_next_free_idx(idx);
        }
        return true;
    }
    bool _check_allocated(int idx) {
        int i = idx_decode(next_free.load());
        int last = -1;
        while( i != -1 ) {
            if( idx == i ) {
                tprintf(tinfra::err, "T[%i]: _check_allocated(%i) fails, because (%i->%i)\n", get_thread_string(), idx, last, idx);
                _check_for_loops();
                return false;
            }
            last = i;
            i = item_next_free_idx(i);
        }
        return true;
    }

    void initialize_storage()
    {
        for( size_t i = 0; i < this->buffer_size; i++ ) {
            item_next_free_idx(i) = i+1;
        }
        item_next_free_idx(this->buffer_size-1) = -1;
        this->next_free = idx_encode(0);
    }
    const int& item_next_free_idx(int i) const {
        return * reinterpret_cast<int const*>(this->buffer_start + i);
    }
    int& item_next_free_idx(int i) {
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
    thread_log_idx = thread_count++;
    const int R = 4192;
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
    for (int n = 0; n < 4; ++n) {
        v.emplace_back(f, n);
    }
    for (auto& t : v) {
        t.join();
    }
    tinfra::tprintf(tinfra::err, "T[main] after test pool size %i\n",pool.allocated());
}

int main()
{
    thread_log_idx = thread_count++;
    test0();
    test1();
    test3();
}