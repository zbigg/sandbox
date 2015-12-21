#ifndef birdblus_h_included
#define birdblus_h_included

#include <type_traits>
#include <functional>
#include <memory>
#include <cassert>
#include <vector>

namespace birdplus {

/*

Promise::resolve(T) -> Promise<T>
Promise::resolve(Promise<T>) -> Promise<T>

Promise<T>::then( T -> R|P<R> ) returns Promise<R>

// note, always allow no-args function to then
// as someone might not be interested in result
Promise<T>::then( void -> R|P<R> ) returns Promise<R>

if R is Promise<R'>, then

// tuple integration, same as spread in bluebird/Q
Promise<tuple<T...>>::spread( T... -> (R|P<R>) -> Promise<R>
// can sugar version of then
Promise<tuple<T...>>::then( T... -> R) -> Promise<R>

// map
Promise::map(iterable<T>, T->R) -> Promise<iterable<R>>
Promise::map(iterable<Promise<T>>, T->R) -> Promise<iterable<R>>

// reduce is interesting
Promise::reduce(iterable<T>, R, (T,R)->R) -> Promise<R>
Promise::reduce(iterable<Promise<T>>, R, (T,R)->R) -> Promise<R>

// Promise.all(Promise<T>...) -> Promise<T...>

*/

template <typename T>
class Promise;

//
// PromisifyType<T>         -> Promise<T>
// PromisifyType<Promise<T> -> Promise<T>
//
template <typename T>
struct PromisifyType {
    typedef Promise<T> type;
};

template <typename T>
struct PromisifyType<Promise<T>> {
    typedef Promise<T> type;
};

static_assert(std::is_same<Promise<int>, PromisifyType<int>::type>::value, "foo");
static_assert(std::is_same<Promise<int>, PromisifyType<Promise<int>>::type>::value, "foo");

namespace detail {
    // this is weirdest thing i've ever seen
    // in C++ WHY THE HELL void is not equal empty arg list !

    template <typename T, typename F, typename V>
    struct resolve_helper_int {
        static void resolve(Promise<T> const& p, F f, V v);
    };

    template <typename T, typename F>
    struct resolve_helper_int<T,F,void> {
        static void resolve(Promise<T> const& p, F f);
    };

    template <typename F, typename V>
    struct resolve_helper_int<void,F,V> {
        static void resolve(Promise<void> const& p, F f, V v);
    };

    template <typename F>
    struct resolve_helper_int<void,F,void> {
        static void resolve(Promise<void> const& p, F f);
    };

    template <typename T,typename F>
    void resolve_helper(Promise<T> p, F f) {
        resolve_helper_int<T,F,void>::resolve(p,f);
    }

    template <typename T,typename F,typename V>
    void resolve_helper(Promise<T> p, F f, V v) {
        resolve_helper_int<T,F,V>::resolve(p,f,v);
    }

}

//
// Promise<T>
//

template <typename T>
class Promise {
    struct SharedState {
        std::function<void(T const&)> callback;
        std::unique_ptr<T> value;

        void resolve(T v) {
            assert(! value );
            if( callback ) {
                callback(v);
            }
            value.reset(new T(std::move(v)));
        }
    };
    std::shared_ptr<SharedState> state;
public:
    Promise(): state(new SharedState) {}
    Promise(Promise<T> const&) = default;
    Promise(Promise<T>&&) = default;

    Promise& operator=(Promise const&) = default;

    Promise<T> const& resolve(T v) const {
        state->resolve(v);
        return *this;
    }

    Promise<T> const& resolve(Promise<T> p) const {
        std::shared_ptr<SharedState> state_ref(state);

        p.then([state_ref](T v) {
            state_ref->resolve(v);
        });

        return *this;
    }
    static T udmmy;
    template <typename F>
    auto then(F f) const -> typename PromisifyType<decltype(f(udmmy))>::type {
        typename PromisifyType<decltype(f(udmmy))>::type result;

        if( state->value) {
            detail::resolve_helper(result,f,*state->value.get());
        } else if( state->callback ) {
            auto old_callback = state->callback;
            state->callback = [=](T const& v) {
                old_callback(v);
                detail::resolve_helper(result,f, v);
            };
        } else {
            state->callback = [=](T const& v) {
                detail::resolve_helper(result,f, v);
            };
        }
        return result;
    }
};

//
// Promise<void>
//
template <>
class Promise<void> {
    struct SharedState {
        std::function<void()> callback;
        bool resolved = false;

        void resolve() {
            assert(!resolved);
            if( callback ) {
                callback();
            }
            resolved = true;
        }
    };
    std::shared_ptr<SharedState> state;
public:
    Promise(): state(new SharedState) {}
    Promise(Promise<void> const&) = default;
    Promise(Promise<void>&&) = default;

    Promise<void> const& resolve() const {
        state->resolve();
        return *this;
    }

    Promise<void> const& resolve(Promise<void> p) const {
        std::shared_ptr<SharedState> state_ref(state);

        p.then([state_ref]() {
            state_ref->resolve();
        });

        return *this;
    }

    template <typename F> // F: () -> R
    auto then(F f) const -> typename PromisifyType<decltype(f())>::type {
        typename PromisifyType<decltype(f())>::type result;

        if( state->resolved) {
            detail::resolve_helper(result,f);
        } else if( state->callback ) {
            auto old_callback = state->callback;
            state->callback = [=]() {
                old_callback();
                detail::resolve_helper(result,f);
            };
        } else {
            state->callback = [=]() {
                detail::resolve_helper(result,f);
            };
        }
        return result;
    }
};

namespace detail {
    // this is weirdest thing i've ever seen
    // in C++ WHY THE HELL void is not equal empty arg list !

    template <typename T, typename F, typename V>
    void resolve_helper_int<T,F,V>::resolve(Promise<T> const& p, F f, V v) {
        p.resolve(f(std::move(v)));
    };

    template <typename T, typename F>
    void resolve_helper_int<T,F,void>::resolve(Promise<T> const& p, F f) {
        p.resolve(f());
    };
    template <typename F, typename V>
    void resolve_helper_int<void,F,V>::resolve(Promise<void> const& p, F f, V v) {
        f(std::move(v));
        p.resolve();
    }

    template <typename F>
    void resolve_helper_int<void,F,void>::resolve(Promise<void> const& p, F f) {
        f();
        p.resolve();
    }

    template <typename T>
    struct to_promise_helper {
        static Promise<T> to_promise(T v) {
            Promise<T> p;
            p.resolve(std::move(v));
            return p;
        }
    };

    template <typename T>
    struct to_promise_helper<Promise<T>> {
        static Promise<T> to_promise(Promise<T> p) {
            return p;
        }
    };
}

//
// to_promise<T>(v) -> Promise<T>
//
// for actual value, return resolved promise
// for promise, just return it
//

template <typename T>
typename PromisifyType<T>::type to_promise(T v)
{
    return detail::to_promise_helper<T>::to_promise(v);
}

template <typename T, typename R, typename F>
Promise<R> reduce(std::vector<Promise<T>> const& list, R initial, F f) {
    if( list.begin() == list.end() ) {
        return to_promise(initial);
    }
    Promise<R> pr = list.begin()->then([f,initial](T const& v) {
        return f(v, initial);
    });
    auto pit = list.begin();
    pit++;
    while( pit != list.end() ) {
        Promise<T> pt = *pit++;
        pr = pr.then([pt,f](R r) -> Promise<R> {
            return pt.then([r,f](T const& v) -> R {
                return f(v, r);
            });
        });
    }
    return pr;
}

template <typename T, typename R, typename F>
std::vector<typename PromisifyType<R>::type> map(std::vector<Promise<T>> const& list, F f) {
    std::vector<typename PromisifyType<R>::type> result;
    result.resize(list.size());
    std::transform(list.begin(), list.end(), result.begin(), [f](Promise<T> const& p) {
        return p.then(f);
    });
    return result;
}

} // end namespace birdplus

#endif // birdblus_h_included