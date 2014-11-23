#include <iostream>

#define PROC() std::cerr << "called " << __func__ << "\n";
#define FUUUN(value) std::cerr << "called " << __func__ << " param: " << (value) << "\n";

//
// normal_recursive_multi_param_func
//

template <typename T>
void my_register(T foo) {
    FUUUN(foo);
}

template <typename T>
void normal_recursive_multi_param_func(T& sig) {
    FUUUN(sig);
}
template <typename T, typename... Rest>
void normal_recursive_multi_param_func(T& sig, Rest&... rest) {
    FUUUN(sig);
    normal_recursive_multi_param_func(rest...);
}

template <typename... Rest>
void try_one(Rest... rest) {
    PROC();
    normal_recursive_multi_param_func(rest...); // ???
}

//
// wrapper attempt
//
/*
template <template <typename> class F>
void call_multi_Rest(F fun)
{
    fun();
}
template <template <typename> class  F, typename T>
void call_multi_Rest(F<T> fun, T&& arg)
{
    fun(std::forward<T>(arg));
}
template <template <typename> class F, typename T, typename... Rest>
void call_multi_args(F<T> fun, T&& arg, Rest&& ... rest)
{
    fun(std::forward<T>(arg));
    call_multi_Rest(fun, std::forward<Rest...>(rest)...);
}
*/
int main()
{
    try_one(1);
    try_one(1,"abc", 2.0);
    //try_two(1,"abc", 2.0);
    
    //call_multi_args(my_register, 1,"abc", 2.0);
}