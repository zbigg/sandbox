#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <string>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <memory>
#include <algorithm>

//
//
//

#define DOUT( aaa ) std::cerr << __func__ << ":" << __LINE__<<  ": " << aaa << "\n";

template <typename T>
class future;

template <>
class future<void>;

template <typename T, typename F>
void set_result_helper(future<void> f, T& val, F fun);

template <typename NT, typename T, typename F>
void set_result_helper(future<NT> f, T& val, F fun);


template <typename F>
void async(F fun);

template <typename T, typename F>
future<T> async(F fun);

//
// optional
//   naive impl based on unique_ptr
// TBD, use some real implementation
//

template <class T>
class optional {
    std::unique_ptr<T> v;
public:
    optional(): v(nullptr) {}
    optional(T&& second): v(new T(second)) {} // TBD, should it be explicit ?
    optional(optional<T> const& second): v( second.get() ? new T(*second.get()) : nullptr) {}

    optional<T>& operator=(optional<T>& second) {
        optional<T> tmp(*this);
        using std::swap;
        swap(*this, second);
    }

    void     reset() { v.release(); }
    void     reset(T value) { v.reset(new T(std::move(value))); }

    operator bool() const { return v.get() != nullptr; };
    bool operator!() const { return v.get() == nullptr; }

    T*       get()       { return v.get(); }
    T const* get() const { return v.get(); }
    T&       ref()       { assert(v); return * v.get(); }
    T const& ref() const { assert(v); return * v.get(); }
    T*       operator ->()       { return get(); }
    T const* operator ->() const { return get(); }
};

template <typename T, typename V>
bool operator &&(optional<T> const& a, optional<V> const& b ) {
    return a.get() && b.get();
}

template <typename T, typename V>
bool operator ||(optional<T> const& a, optional<V> const& b ) {
    return a.get() || b.get();
}

template <typename T, typename V>
bool operator ==(optional<T> const& a, optional<V> const& b ) {
    return ( a && b )   ? ( a.ref() == b.ref() ) :
           ( !a && !b ) ? true
                        : false;
}

template <typename T, typename V>
bool operator !=(optional<T> const& a, optional<V> const& b ) {
    return ( a && b )   ? ( a.ref() != b.ref() ) :
           ( !a && !b ) ? false
                        : true;
}

template <typename T, typename V>
bool operator ==(optional<T> const& a, V const& b ) {
    return a.get() && a.ref() == b;
}

template <typename T, typename V>
bool operator !=(optional<T> const& a, V const& b ) {
    return !a.get() || a.ref() != b;
}

template <typename T, typename V>
bool operator ==(T const& a, optional<V> const& b ) {
    return b.get() && b.ref() == a;
}

template <typename T, typename V>
bool operator !=(T const& a, optional<V> const& b ) {
    return !b.get() || b.ref() != a;

}

//
// future & async
//

template <>
class future<void> {
    struct future_state {
        bool resolved;
        std::function<void(void)> consume;
    };
    std::shared_ptr<future_state> state;
public:
    future(): state(new future_state() ) { }

    template <typename F, typename TT>
    void consume(F f, TT t) const
    {
        f(t);
        consume();
    }
    void consume() const {
        assert(!state->resolved);

        if( !state->consume ) {
            state->resolved = true;
            return;
        }
        std::shared_ptr<future_state> cstate(state);
        async([cstate]() {
            // DOUT("future("<<cstate<<") -> calling consume");
            cstate->consume();
        });
    }

    // I emit void
    // f shall be () -> ANY
    template <typename F>
    auto then(F f) -> future<decltype(f())> {
        future<decltype(f())> r;

        if( this->state->resolved ) {
            r.consume(f());
        } else if( this->state->consume) {
            auto old_consume = this->state->consume;
            this->state->consume = [=]() {
                old_consume();
                r.consume(f());
            };
        } else {
            this->state->consume = [=]() {
                this->state->resolved = true;
                r.consume(f());
            };
        }
        return r;
    }
};


template <typename T>
class future {
    struct future_state {
        optional<T> value;
        std::function<void(T)> consume;
    };
    std::shared_ptr<future_state> state;
    static T dummy;

public:
    future(): state(new future_state() ) { }

    template <typename F, typename TT>
    void consume(F f, TT t) const
    {
        consume(f(t));
    }
    void consume(T v) const {
        // DOUT("future::set_result<T>, T=" << typeid(T).name());
        assert( !this->state->value );
            // TBD, second value !???

        if( !state->consume ) {
            this->state->value.reset(v);
            return;
        }

        assert( !!state->consume );
        std::shared_ptr<future_state> cstate(state);
        async([cstate,v]() {
            // DOUT("future T=" << typeid(T).name() << "("<<cstate<<") -> calling consume");
            cstate->consume(v);
        });
    }

    // I emit T
    // f shall be (T) -> ANY
    template <typename F>
    auto then(F f) -> future<decltype(f(dummy))> {
        using NT = decltype(f(dummy));
        future<NT> r;
        // DOUT("future T=" << typeid(T).name() << "("<<this->state<<") -> setting consumer");
        if( this->state->value ) {
            r.consume(f, this->state->value.ref());
        } else if( this->state->consume ) {
            auto old_consume = this->state->consume;
            this->state->consume = [=](T v) {
                old_consume(v);
                r.consume(f, v);
            };
        } else {
            this->state->consume = [=](T v) {
                this->state->value.reset(v);
                r.consume(f, v);
            };
        }
        return r;
    }
};

template <typename NT, typename T, typename F>
void set_result_helper(future<NT> f, T& val, F fun)
{
    f.consume(fun(val));
}

template <typename T, typename F>
void set_result_helper(future<void> f, T& val, F fun)
{
    fun(val);
    f.consume();
}

//
// async impl
//
std::queue<std::function<void()>> async_queue;
std::mutex async_mutex;
std::condition_variable async_cond;
bool quit = false;

void async_loop()
{
    while( !quit ) {
        std::function<void()> next;
        {
            std::unique_lock<std::mutex> lock(async_mutex);

            if( async_queue.empty() ) {
                // DOUT("async_wait");
                async_cond.wait(lock);
                continue;
            }
            std::swap(async_queue.front(), next);
            async_queue.pop();
        }
        // DOUT("async::call");
        next();
    }
    // DOUT("async_quit");
}
template <typename T, typename F>
future<T> async_future(F fun) {
    future<T> f;
    // DOUT("async::push future");
    std::lock_guard<std::mutex> lock(async_mutex);

    async_queue.push([=]() {
        try {
            f.consume( fun() );
        } catch( ... ) {
            // TBD, f.set_exception not implemented
            std::cerr << "async: exception_caught, not propagatinh, abortinh aaaa\n";
            throw;
        }
    });
    return f;
};

template <typename F>
void async(F fun) {
    // DOUT("async::push");
    std::lock_guard<std::mutex> lock(async_mutex);

    async_queue.push([=]() {
        fun();
    });
};

void async_quit()
{
    async([]() { 
        DOUT("#13");
        quit = true; 
        });
}

//
// the code
//
future<std::string> read_content(std::string name)
{
    return async_future<std::string>([=]() -> std::string {
        DOUT("rr #9");
        return "trompka pompka";
    });
}

int main()
{
    auto fc = read_content("foo");

    fc.then([](std::string content) -> std::vector<std::string> {
        DOUT("aa #10");
        std::vector<std::string> r;
        r.push_back(content);
        return r;
    })
    .then([](std::vector<std::string> v) -> int {
        DOUT("aa #11");
        return v.size();
    })
    .then([](int v)  {
        DOUT("aa #12");
        std::cout << "foo: " << v << "\n";
        async_quit();
    });

    fc.then([](std::string content) {
        DOUT("sc #1");
    }).then([]() -> int {
        DOUT("sc #2 void -> int");
        return 666;
    }).then([](int a) {
        DOUT("sc #3, done");
    });

    async_loop();
}
