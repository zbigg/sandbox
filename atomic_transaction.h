

template <typename T, typename Rest...>
class atomic_transaction:  public atomic_transaction<Rest...> {
private:
    T* rv;
    T  copy;
    atomic_transaction(T& v, Rest... args):
        atomic_transaction(Rest...),
        rv(v),
        copy(
    {
    }
};