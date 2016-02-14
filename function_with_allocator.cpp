#include <functional>
#include <iostream>
#include <cassert>

//
// allocators
//
// TBD, see Alexandrescu video about allocators again

struct allocator {
    virtual ~allocator() {}

    virtual void* alloc(size_t size) = 0;
    virtual void  free(void* p) = 0;
};

/// Small Object Optimization-enabled single object allocator
///
/// Useful as base for buffer
/// 3 lifecycle options
/// 
/// managed construct/destruct:
///   T* b.template create<T>(constructor_args)
///   b.template cleanup<T>();
///
/// only allocate/deallocate using allocator interface
///   void* p = b.alloc(SIZE);
///   b.free(p)
///
/// TBD:
///   parent_allocator as template
///
///  that would be very cool design!
/// TBD
template <int static_size = 16> 
struct soo_buffer: public allocator {
    union {
        void*                      ptr;
        typename std::aligned_storage<static_size>::type buf;
    };
    enum  {
        EMPTY,
        DYNAMIC,
        STATIC
    } state;

    soo_buffer(): state(EMPTY) {}

    soo_buffer(soo_buffer<static_size>&& other):
        state(other.state)
        // move construction -> move only what needed
    {
        switch(other.state) {
        case DYNAMIC:
            ptr = other.ptr;
            other.ptr = nullptr;
            break;
        case DYNAMIC:
            buf = other.buf;
            break;
        case EMPTY:
            break;
        }
        other.state = EMPTY;
    }

    ~soo_buffer() 
    {
        assert(!initialized());
    }

    // allocator interface
    void* alloc(size_t size) override {
        return _allocate(size);
    }

    void free( void* p ) override
    {
        if( !p ) return;

        assert( initialized() );
        assert( (state == DYNAMIC && p == ptr ) ||
                (state == STATIC && p == &buf ));
        state = EMPTY;
    }

    // soo_buffer interface
    bool initialized() const { return (state != EMPTY); }

    template <typename T, typename... Args>
    void create(Args&&... args) {
        assert(!initialized());
        new(_allocate<T>()) T(std::forward<Args&&...>(args)...);
    }

    void* _allocate(std::size_t size) {
        assert(!initialized());
        if( size > static_size) {
            std::cerr << typeid(*this).name() << ": dynamic, " << size << "\n";
            state = DYNAMIC;
            ptr = std::malloc(size);
            return ptr;
        } else {
            std::cerr << typeid(*this).name() << ": static, " << size << "\n";
            state = STATIC;
            return &buf;
        }
    }

    template <typename T>
    T* _allocate() {
        return reinterpret_cast<T*>(_allocate(sizeof(T)));
    }

    void* get() {
        if( state == DYNAMIC ) {
            return ptr;
        } else {
            return &buf;
        }
    }

    const void* get() const {
        if( state == DYNAMIC ) {
            return ptr;
        } else {
            return &buf;
        }
    }

    template <typename T>
    T* get() {
        assert(initialized());
        return reinterpret_cast<T*>(get());
    }

    template <typename T>
    T const* get() const {
        assert(initialized());
        return reinterpret_cast<T const*>(get());
    }

    template <typename T>
    void cleanup() {
        if( !initialized() ) {
            return;
        }
        get<T>()->~T();
        if( state == DYNAMIC ) {
            std::free(ptr);
        }
        state = EMPTY;
    }

    void release()
        /// release object, even if
        /// it was not constructed
    {
        state = EMPTY;
    }
};

template <typename Base, int static_size = 16>
class soo_unique_ptr {
    soo_buffer<static_size> v;
public:
    // default construction -> empty object
    soo_unique_ptr() {}

    // move construction -> move only what needed
    soo_unique_ptr(soo_unique_ptr&& other):
        v(std::move(other.v))
    { }

    // construct with Derived from copy
    template <typename Derived>
    soo_unique_ptr(Derived d)
    {
        v.template create<Derived, Derived&&>(std::move(d));
    }

    ~soo_unique_ptr()
    {
        v.template cleanup<Base>();
    }

    //
    // my interface
    //

    // reset with new instance of Derived
    // equal to unique_ptr.reset(new Derived(args...))
    template <typename Derived, typename... Args>
    void reset_new(Args&&... args)
    {
        v.template cleanup<Base>();
        v.template create<Derived, Args...>(std::forward<Args&&...>(args)...);
    }


    // reset with clone of some Derived instance
    // assumes Base exposes:
    //    Bas* clone(allocator*) const
    void reset_clone(Base const* d) 
    {
        v.template cleanup<Base>();
        d->clone( &v );
        assert( v.initialized() );
    }

    Base* get() { return v.template get<Base>(); }
    Base const* get() const { return v.template get<Base>(); }

    // unique_ptr interface
    Base& operator*() { return *get(); }
    Base const& operator*() const { return *get(); }

    Base* operator->() { return get(); }
    Base const* operator->() const { return get(); }
};

//
// xfunction
//

template <typename R, typename... Args>
struct fun2 {
    virtual ~fun2() {}
    virtual R operator()(Args&&... args) = 0;
    virtual fun2<R,Args...>* clone(allocator* alloc = nullptr) const = 0;
};

template <typename F, typename R, typename... Args>
class fun2_impl: public fun2<R,Args...> {
    F target;
public:
    fun2_impl(F&& t): target(t) {}

    R operator()(Args&&... args) override {
        return target(std::forward<Args&&...>(args)...);
    }

    fun2<R,Args...>* clone(allocator* a) const override {
        F tmp(target);
        if( a ) {
            void* mem = a->alloc(sizeof(fun2_impl<F,R,Args...>));
            return new(mem) fun2_impl<F,R,Args...>(std::move(tmp));
        } else {
            return new fun2_impl<F,R,Args...>(std::move(tmp));
        }
    }
};

template <typename NA>
class fun;

template <typename R, typename... Args>
class fun<R(Args...)> {
    // std::unique_ptr<fun2<R,Args...>> impl;
    
    soo_unique_ptr<fun2<R,Args...>, 24> impl;
public:
    /*
    template <typename F>
    fun(F t):
        impl(new fun2_impl<F,R,Args...>(std::move(t))) { }

    fun(fun<R(Args...)>&& other):
        impl(std::move(other.impl)) { }

    fun(fun<R(Args...)> const& other):
        impl(other.impl ? other.impl->clone() : nullptr) { }

    fun<R(Args...)>& operator=(fun<R(Args...)> const& other) 
    {
        impl.reset(other.impl ? other.impl->clone() : nullptr);
        return *this;
    }
    */

    template <typename F>
    fun(F t) 
    {
        impl.template reset_new<fun2_impl<F,R,Args...>>(std::move(t));
    }

    fun(fun<R(Args...)> const& other)
    {
        impl.reset_clone(other.impl.get());
    }
    fun(fun<R(Args...)>&& other):
        impl(std::move(other.impl))
    {
    }

    fun<R(Args...)>& operator=(fun<R(Args...)> const& other) 
    {
        impl.reset_clone(other.impl);
        return *this;
    }

    R operator()(Args&&... args) 
    {
        // assert(impl != nullptr);
        return (*impl)(std::forward<Args&&...>(args)...);
    }
};

int main() {
    std::cerr << "std::function<>" << sizeof(std::function<int(int)>) << "\n";
    std::cerr << "fun<>" << sizeof(fun<int(int)>) << "\n";
    fun<int(int)> rabarbar([](int a)->int {
        std::cerr << "#2\n";
        return a*2;
    });
    std::string dupa = "dupa";

    fun<void(int)> fkejo([&dupa,&rabarbar](int a)->void {
        std::cerr << "#4 " << dupa << "\n";
    });
    fkejo(3);

    auto x = rabarbar;
    int r = rabarbar(2);
    std::cerr << "#3" << r << "\n";

    /*auto boo = make_holder([](int a) {
        std::cerr << "#1\n";
    });
    boo(2);*/
}