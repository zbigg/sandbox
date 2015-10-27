#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <string>
#include <cassert>
#include <condition_variable>

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
// future & async
//

template <>
class future<void> {
    struct future_state {
        std::function<void(void)> consume;
    };
    std::shared_ptr<future_state> state;
public:
    future(): state(new future_state() ) { }

    template <typename F, typename TT>
    void set_result(F f, TT t) const
    {
        f(t);
        set_result();
    }
    void set_result() const {
        // DOUT("future::set_result<void>");
        if( !state->consume ) {
            // DOUT("future::set_result<void> -> abandoning");
            return;
        }
        std::shared_ptr<future_state> cstate(state);
        async([cstate]() {
            // DOUT("future("<<cstate<<") -> calling consume");
            cstate->consume();
        });
    }

    template <typename F>
    auto then(F f) -> future<decltype(f())> {
        future<decltype(f())> r(f);
        // DOUT("future("<<this->state<<") -> setting consumer");
        this->state->consume = [=]() {
            r.set_value();
        };
        return r;
    }
};


template <typename T>
class future {
    struct future_state {
        std::function<void(T)> consume;
    };
    std::shared_ptr<future_state> state;
    static T dummy;

public:
    future(): state(new future_state() ) { }

    template <typename F, typename TT>
    void set_result(F f, TT t) const
    {
        set_result(f(t));
    }
    void set_result(T v) const {
        // DOUT("future::set_result<T>, T=" << typeid(T).name());
        if( !state->consume ) {
            // DOUT("future::set_result<T>, T=" << typeid(T).name() <<" -> abandoning");
            return;
        }

        assert( !!state->consume );
        std::shared_ptr<future_state> cstate(state);
        async([cstate,v]() {
            // DOUT("future T=" << typeid(T).name() << "("<<cstate<<") -> calling consume");
            cstate->consume(v);
        });
    }

    template <typename F>
    auto then(F f) -> future<decltype(f(dummy))> {
        using NT = decltype(f(dummy));
        future<NT> r;
        // DOUT("future T=" << typeid(T).name() << "("<<this->state<<") -> setting consumer");
        this->state->consume = [=](T v) {
            // r.set_result(f(v));
            r.set_result(f, v);
        };
        return r;
    }
};

template <typename NT, typename T, typename F>
void set_result_helper(future<NT> f, T& val, F fun)
{
    f.set_result(fun(val));
}

template <typename T, typename F>
void set_result_helper(future<void> f, T& val, F fun)
{
    fun(val);
    f.set_result();
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
            f.set_result( fun() );
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
        DOUT("#9");
        return "trompka pompka";
    });
}

int main()
{
    read_content("foo")
        .then([](std::string content) -> std::vector<std::string> {
            DOUT("#10");
            std::vector<std::string> r;
            // parse content
            return r;
        })
        .then([](std::vector<std::string> v) -> int {
            DOUT("#11");
            return v.size();
        })
        .then([](int v)  {
            DOUT("#12");
            std::cout << "foo: " << v << "\n";
            async_quit();
        });

    async_loop();
}
