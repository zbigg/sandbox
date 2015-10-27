#include <iostream>
#include <sys/time.h>

#define DOUT( aaa ) std::cerr << __func__ << ":" << __LINE__<<  ": " << aaa << "\n";

struct timer {
    struct timeval start;
    timer() {
        gettimeofday(&start, nullptr);
    }
    ~timer() {
        struct timeval end;
        gettimeofday(&end, nullptr);
        std::cout << ((long long)end.tv_sec * 1000000 + end.tv_usec ) - ((long long)start.tv_sec * 1000000 + start.tv_usec) << "\n";
    }
};

template <typename F>
auto timeit(F f) -> decltype(f())
{
    timer t;
    return f();
}

int main()
{

    timeit([=]{
        int x = 0;
        for(int i = 0; i < 100000000; ++i ) {
            x++;
        }
    });
}
