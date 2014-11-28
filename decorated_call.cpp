#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <utility>
#include <tuple>
#include <iostream>

template<std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
  xfor_each(std::tuple<Tp...> &, FuncT) // Unused arguments are given no names.
  { }

template<std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
  xfor_each(std::tuple<Tp...>& t, FuncT f)
  {
    f(std::get<I>(t));
    xfor_each<I + 1, FuncT, Tp...>(t, f);
  }

template <typename... Decorators>
struct decorated_runner {
private:

    struct decorator_starter {
        template <typename T>
        void operator()(T&& t) const { t.start(); }
    };

    struct decorator_stopper {
        template <typename T>
        void operator()(T&& t) const { t.stop(); }
    };

    std::tuple<Decorators...> ddd;
public:
    template <typename F, typename... T>
    void execute_decorated(F fun, T&&... args) {
        xfor_each(ddd, decorator_starter{});

        fun(std::forward<T&&...>(args)...);

        xfor_each(ddd, decorator_stopper{});
    }
};

struct static_run_calculator {

    static int starts;
    static int ends;

    void start() {
        std::cout << __PRETTY_FUNCTION__ << "\n";
        ++starts;
    }

    void stop() {
        std::cout << __PRETTY_FUNCTION__ << "\n";
        ++ends;
    }
};

int static_run_calculator::starts = 0;
int static_run_calculator::ends = 0;

struct time_calculator {

    time_t t0;
    time_t t1;

    void start() {
        std::cout << __PRETTY_FUNCTION__ << "\n";
        ::time(&t0);
    }
    void stop() {
        std::cout << __PRETTY_FUNCTION__ << "\n";
        ::time(&t1);
    }

};

void sleeepy() {
    std::cout << __PRETTY_FUNCTION__ << "\n";
    ::sleep(2);
}
int main()
{
    decorated_runner<static_run_calculator, time_calculator> r;

    r.execute_decorated(sleeepy);
}



