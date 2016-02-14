#ifndef simple_async_engine_h_included
#define simple_async_engine_h_included

#include <queue>

///
/// Simple async engine
///
/// Single threaded async engine that queues
/// callbacks and calls them in order from main loop.
///
class task_queue {
    //
    // async impl
    //
    std::queue<std::function<void()>> queue;
    // std::mutex async_mutex;
    // std::condition_variable async_cond;
public:
    task_queue()
    {
    }
    ~task_queue()
    {
    }

    void execute_tasks()
    {
        while( true ) {
            std::function<void()> next;
            
            // std::lock_guard<std::mutex> lock(async_mutex);
            // {
            if( queue.empty() ) {
                return;
            }
            std::swap(queue.front(), next);
            queue.pop();
            // }

            // DOUT("async::call");
            next();
        }
        // DOUT("async_quit");
    }
    /*
    template <typename T, typename F>
    future<T> async_future(F fun) {
        future<T> f;
        // DOUT("async::push future");
        // std::lock_guard<std::mutex> lock(async_mutex);
    
        queue.push([=]() {
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
    */

    bool empty() const {
        return this->queue.empty();
    }

    template <typename Fun, typename... Args>
    void push(Fun f, Args&&... args) {
        // DOUT("async::push");
        // std::lock_guard<std::mutex> lock(async_mutex);

        this->queue.push([=]() {
            f(std::forward<Args&&...>(args)...);
        });
    };

    template <typename Fun, typename... Args>
    void operator()(Fun f, Args&&... args) {
        this->push(f, std::forward<Args&&...>(args)...);
    }
};

#endif // simple_async_engine_h_included
