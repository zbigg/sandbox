#include "tinfra/test.h"
#include "birdplus.h"

#include <exception>

struct reached_mark {
    bool reached;
    reached_mark(): reached(false) { }
    ~reached_mark() {
        CHECK_EQUAL(reached, true);
    }
};

#define MARK_REACHED(f) f.reached = true

#define CHECK_STD_EXCEPTION_WHAT(exception_type, expected_what_value, eptr) \
    do { \
        try { \
            std::rethrow_exception(eptr); \
        } catch( exception_type& e ) { \
            CHECK_EQUAL(expected_what_value, e.what()); \
        } catch( ... ) { \
            CHECK(false); \
        } \
    } while(false)

SUITE(promise) {
    using birdplus::to_promise;
    using birdplus::Promise;

    //
    // Promise/A/C++ API
    //
    //  Promise<type>::resolve(...) as Promise.resolve()
    //  new Promise<type>(executor) as new Promise(executor)
    //
    TEST(promise_a_cpp_resolve) {
        Promise<int>::resolve(5)
            .then([](int v) {
                CHECK_EQUAL(5, v);
            });
        bool reached = false;

        Promise<void>::resolve()
            .then([&reached]() {
                reached = true;
            });
        CHECK(reached);
    }
    
    TEST(promise_a_cpp_new_promise) {
        bool reached_constructor = false;
        auto p = std::make_unique<Promise<int>>([&](auto resolve, auto reject) {
            reached_constructor = true;
            resolve(55);
        });
        CHECK(reached_constructor);

        bool resolved_reached = false;
        p->then([&](int v) {
            resolved_reached = true;
            CHECK_EQUAL(55, v);
        });
        CHECK(resolved_reached);
    }
    TEST(promise_a_cpp_new_promise_fail) {
        bool reached_constructor = false;
        auto p = std::make_unique<Promise<int>>([&](auto resolve, auto reject) {
            reached_constructor = true;
            throw std::runtime_error("fail");
            CHECK(false);
        });
        CHECK(reached_constructor);

        bool resolved_reached = false;
        bool error_reached = false;
        bool error_second_flow_reached = false;
        p->then([&](int v) {
            resolved_reached = true;
            CHECK(false);
        }).error([&](std::exception_ptr eptr) {
            error_second_flow_reached = true;
            CHECK_STD_EXCEPTION_WHAT(std::exception, "fail", eptr);
        });
        p->error([&](std::exception_ptr eptr) {
            error_reached = true;
            CHECK_STD_EXCEPTION_WHAT(std::exception, "fail", eptr);
        });
        CHECK(!resolved_reached);
        CHECK(error_reached);
        CHECK(error_second_flow_reached);
    }
    TEST(promise_a_cpp_new_void_promise) {
        bool reached_constructor = false;
        auto p = std::make_unique<Promise<void>>([&](auto resolve, auto reject) {
            reached_constructor = true;
            resolve();
        });
        CHECK(reached_constructor);

        bool resolved_reached = false;
        p->then([&]() {
            resolved_reached = true;
        });
        CHECK(resolved_reached);
    }
    TEST(promise_a_cpp_new_void_promise_fail) {
        bool reached_constructor = false;
        auto p = std::make_unique<Promise<void>>([&](auto resolve, auto reject) {
            reached_constructor = true;
            throw std::runtime_error("fail");
            CHECK(false);
        });
        CHECK(reached_constructor);

        bool resolved_reached = false;
        bool error_reached = false;
        bool error_second_flow_reached = false;
        p->then([&]() {
            resolved_reached = true;
            CHECK(false);
        }).error([&](std::exception_ptr eptr) {
            error_second_flow_reached = true;
            CHECK_STD_EXCEPTION_WHAT(std::exception, "fail", eptr);
        });
        p->error([&](std::exception_ptr eptr) {
            error_reached = true;
            CHECK_STD_EXCEPTION_WHAT(std::exception, "fail", eptr);
        });
        CHECK(!resolved_reached);
        CHECK(error_reached);
        CHECK(error_second_flow_reached);
    }

    TEST(empty_promise) {
        reached_mark mark;
        to_promise(22).then([&mark](int v) {
            CHECK_EQUAL(22, v);
            MARK_REACHED(mark);
        });
        // my_async::wait();
    }

    TEST(reduce) {
        reached_mark mark;
        std::vector<Promise<int>> vec;
        vec.push_back(to_promise<int>(1));
        vec.push_back(to_promise<int>(2));
        vec.push_back(to_promise<int>(3));
        vec.push_back(to_promise<int>(4));

        using birdplus::reduce;
        int i = 0;
        reduce(vec, 690, [&i](int v, int acc) -> int {
            i++;
            return acc+v;
        }).then([&mark,&i](int result) {
            CHECK_EQUAL(4, i);
            CHECK_EQUAL(700, result);
            MARK_REACHED(mark);
        });
        // my_async::wait();
    }

    TEST(map) {
        std::vector<Promise<int>> vec;
        vec.push_back(to_promise<int>(1));
        vec.push_back(to_promise<int>(2));
        vec.push_back(to_promise<int>(3));
        vec.push_back(to_promise<int>(4));

        using birdplus::map;
        auto result = map<int,int>(vec, [](int v) -> int {
            return 2*v;
        });

        result[0].then([](int r) { CHECK_EQUAL(2, r); });
        result[1].then([](int r) { CHECK_EQUAL(4, r); });
        result[2].then([](int r) { CHECK_EQUAL(6, r); });
        result[3].then([](int r) { CHECK_EQUAL(8, r); });
        // my_async::wait();
    }

    TEST(exception_catch) {
        to_promise<int>(22).then([](int r) -> int {
            CHECK_EQUAL(22, r);
            throw 33;
        }).then([](int) {
            // not reached
            CHECK(false);
        }).error([](std::exception_ptr e) {
            try {
                std::rethrow_exception(e);
            } catch(int a) {
                CHECK_EQUAL(33, a);
            } catch( ... ) {
                CHECK(false);
            }
        });
    }
}

int main(int argc, char** argv)
{
    tinfra::public_tracer::process_params(argc, argv);

    return tinfra::test::test_main(argc, argv);
}