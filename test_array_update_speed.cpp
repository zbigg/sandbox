#include <utility>
#include <vector>
#include <iostream>
#include <chrono>
#include <string.h>

//
// getrusage test decorator
//


#include <sys/time.h>
#include <sys/resource.h>

struct test_decorator {
    virtual ~test_decorator() {}

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void output(test_result_sink& sink) = 0;
};
struct test_result_sink {
    const char* test_name;

    void start() {
    }
    template <typename T>
    void output(const char* name, T value) {
        std::cout << test_name << "/" << name << " " << value << "\n";
    }
};
class hires_clock_run_analyzer {
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point stop_time;
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    void stop() {
        stop_time = std::chrono::high_resolution_clock::now();
    }

    void output(test_result_sink& sink) {
        using std::chrono::duration;
        using std::chrono::duration_cast;
        const duration<double> time_span = duration_cast<duration<double>>(stop_time - start_time);

        sink.output("hiperf_time", time_span.count());
    }
};
class rusage_run_analyzer {
    struct rusage before;
    struct rusage after;
public:
    void start() {
        memset(&this->before, 0, sizeof(struct rusage));
        memset(&this->after, 0, sizeof(struct rusage));
        getrusage(RUSAGE_SELF, &this->before);
    }
    void stop() {
        getrusage(RUSAGE_SELF, &this->after);
    }
    void output(test_result_sink& sink) {
        // user time
        const long   user_cpu_seconds =      (this->after.ru_utime.tv_sec - this->before.ru_utime.tv_sec);
        const long   user_cpu_microseconds = (this->after.ru_utime.tv_usec - this->before.ru_utime.tv_usec);
        const double user_cpu_time = (double)user_cpu_seconds + (double) user_cpu_microseconds / 1000000.0;
        sink.output("rusage_user_cpu_seconds", user_cpu_seconds);
        sink.output("rusage_user_cpu_microseconds", user_cpu_microseconds);
        sink.output("rusage_user_cpu_time", user_cpu_time);

        // system_time
        const long   system_cpu_seconds      = (this->after.ru_stime.tv_sec - this->before.ru_stime.tv_sec);
        const long   system_cpu_microseconds = (this->after.ru_stime.tv_usec - this->before.ru_stime.tv_usec);
        const double system_cpu_time = (double)system_cpu_seconds + (double) system_cpu_microseconds / 1000000.0;
        sink.output("rusage_system_cpu_seconds", system_cpu_seconds);
        sink.output("rusage_system_cpu_microseconds", system_cpu_microseconds);
        sink.output("rusage_system_cpu_time", system_cpu_time);

        // page faults
        sink.output("rusage_hard_page_faults", (this->after.ru_majflt - this->before.ru_majflt));
        sink.output("rusage_soft_page_faults", (this->after.ru_minflt - this->before.ru_minflt));

        // i/o
        sink.output("rusage_io_reads", (this->after.ru_inblock - this->before.ru_inblock));
        sink.output("rusage_io_writes", (this->after.ru_oublock  - this->before.ru_oublock));

        // scheduling
        sink.output("rusage_non_voluntary_context_switches", (this->after.ru_nivcsw - this->before.ru_nivcsw));
        sink.output("rusage_voluntary_context_switches", (this->after.ru_nvcsw  - this->before.ru_nvcsw));

        // memory
        sink.output("rusage_maxrss_delta", (this->after.ru_maxrss - this->before.ru_maxrss));
    }
};
void test_column_wise(std::vector<int>& foo, int rows, int cols)
{
    for(int i = 0; i < cols; ++i ) {
        for(int j = 0; j < rows; ++j ) {
            foo[j*cols + i] += 42;
        }
    }
}

void test_row_wise(std::vector<int>& foo, int rows, int cols)
{
    //       <---- cols --- >
    //  ^    [ a b c d e f g h j ... ]
    // rows  [ ...                   ]
    //  v    [                       ]

    for(int j = 0; j < rows; ++j ) {
        for(int i = 0; i < cols; ++i ) {
            foo[j*cols + i] += 42;
        }
    }
}

template <typename F, typename ...Args>
void run_test(test_result_sink& sink, F fun, Args&&... args)
{
    hires_clock_run_analyzer hires_clock;
    rusage_run_analyzer rusage;

    sink.start();
    rusage.start();
    hires_clock.start();

    bool success = true;
    const int repeats = 1000;
    for(int i = 0; i < repeats; ++i ) {
        try {
            fun(std::forward<Args>(args)...);
        } catch( std::exception& e ) {
            success = false;
        }
    }
    //sink.end(success);
    hires_clock.stop();
    rusage.stop();
    sink.output("success", success);

    hires_clock.output(sink);
    rusage.output(sink);
}

template <typename F, typename ...Args>
void run_test2(const char* name, F fun, Args&&... args)
{
    test_result_sink sink = { name };
    run_test(sink, fun, std::forward<Args>(args)...);
}

int main()
{
    const int rows = 1000;
    const int cols = 1000;
    std::vector<int> foo(rows*cols, 0);
    std::vector<int> boo(rows*cols, 0);

    run_test2("test_column_wise", test_column_wise, foo, rows, cols);
    run_test2("test_row_wise", test_row_wise, boo, rows, cols);
}
