#include "birdplus.h"
#include <iostream>

int main()
{
    using birdplus::to_promise;
    using birdplus::Promise;

    to_promise(22).then([](int v) {
        std::cout << "hello: " << v << "\n";
    });

    std::vector<Promise<int>> vec;
    vec.push_back(to_promise<int>(1));
    vec.push_back(to_promise<int>(2));
    vec.push_back(to_promise<int>(3));
    vec.push_back(to_promise<int>(4));

    using birdplus::map;
    using birdplus::reduce;

    reduce(map<int,int>(vec, [](int v) -> int {
        // throw 3;
        return 2*v;
    }), 0, [](int v, int acc) -> int {
        std::cerr << "bbb!\n";
        // throw 2;
        return acc+v;
    }).then([](int result) {
        std::cerr << "aaa!\n";
        throw 1;
        std::cout << "reduce: " << result << "\n";
    }).error([](std::exception_ptr eptr) {
        std::cerr << "error caought" << "\n";
        try {
            std::rethrow_exception(eptr);
        } catch(int a) {
            std::cerr << "error: " << a << "\n";
        }
    });
}