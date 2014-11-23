template <typename T>
struct signal;

struct signal_system {
    template <typename T>
    void emit_value(signal<T>& signal, T value) {
    }
};

template <typename T>
struct signal {
        T current_value;
        T previous_value;
        
        
        
        virtual void actuate(T value)
        {
        }
        T& operator = (T value) {
            signal_system::emit_value(*this, std::move(value));
        }
};

template <typename T>
struct fun_signal {
    
    template <typename... Signals>
    fun_signal(Signals&... sigs) {
        register_sigs(sigs)...; // ???
        register_sigs(sigs...)...; // ???
    }
    template <typename T.
    void register_sigs(T& sig) {
        signal_system::connect(*this, sig)
    }
    template <typename... Signals>
    void register_sigs(T& sig, Signals&... sigs) {
        signal_system::connect(*this, sig);
        this->register_sigs(sigs)...;
    }
    fun_signal& c
};

singal<int> width = 5;
signal<int> height = 7;

signal<int> area = fun_signal<int>(width, height).with([](int w, int h) {
        return w * h;
        });


/*int main()
{
    
}
*/


