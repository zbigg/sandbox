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
        return 2*v;
    }), 0, [](int v, int acc) -> int {
        return acc+v;
    }).then([](int result) {
        std::cout << "reduce: " << result << "\n";
    });
}