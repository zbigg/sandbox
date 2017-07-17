#include <uv.h>

#include <hiredis/async.h>
#include <hiredis/adapters/libuv.h>

#include "birdplus.h"

#include <cstddef>

#include <map>
#include <vector>
#include <string>
#include <exception>

using birdplus::Promise;

struct RedisConnection {

    struct Reply {
        int type;
        std::string str;
        int integer;
        std::vector<Reply> elements;
    };

    RedisConnection():
        ac(redisAsyncConnect("127.0.0.1", 6379)),
        next_id(1)
    {
        std::cerr << "redisAsyncConnect connecting\n";
        if( !ac || ac->err ) {
            std::cerr << "redisAsyncConnect failed " << ac->err << ", " << ac->errstr << "\n";
        }
        redisAsyncSetConnectCallback(this->ac, [](const redisAsyncContext *c, int status) {
            // TBD map it etc ...
            std::cerr << "RedisConnection connected " << status << "\n";
        });
        redisAsyncSetDisconnectCallback(this->ac, [](const redisAsyncContext *c, int status) {
            // TBD map it etc ...
            std::cerr << "RedisConnection disconnected " << status << "\n";
        });
    }
    Promise<Reply> exec(const char* format, ...) {
        int request_id = this->next_id++;
        request_entry& re = this->pending_requests[request_id];
        re.id = request_id;
        re.conn = this;


        Promise<Reply> result;
        auto callback = [](redisAsyncContext* ac, void* reply, void* data) {
            std::cerr << "#exec callback\n";
            request_entry* re = reinterpret_cast<request_entry*>(data);
            if( !reply ) {
                re->promise.reject(std::runtime_error("#redisAsyncCommandArgv null reply"));
            } else {
                Reply r = makeReply(reinterpret_cast<redisReply*>(reply));
                std::cerr << "xxx\n";
                re->promise.settle(r);
            }
            re->conn->pending_requests.erase(re->id);
        };
        std::cerr << "#exec " << format << "\n";
        va_list ap;
        va_start(ap, format);
        int r = redisvAsyncCommand(this->ac, callback, &re, format, ap);
        va_end(ap);
        if( r != REDIS_OK ) {
            throw std::runtime_error("#redisAsyncCommandArgv failed");
        }
        re.promise = result;
        return result;
    }

    static Reply makeReply(redisReply* reply) {
        struct Reply result;
        std::cerr << "#makeReply ... " << reply->type << "\n";
        result.type = reply->type;
        if( result.type == REDIS_REPLY_STRING || result.type == REDIS_REPLY_STATUS ) {
            result.str.assign(reply->str, reply->len);
        } else if( result.type == REDIS_REPLY_INTEGER ) {
            result.integer = reply->integer;
        } else if( result.type == REDIS_REPLY_ARRAY ) {
            result.elements.resize(reply->elements);
            for( size_t i = 0; i < reply->elements; ++i ) {
                result.elements[i] = makeReply(reply->element[i]);
            }
        } else if( result.type == REDIS_REPLY_ERROR ) {
            std::string message = std::string("#redis error reply: ") + reply->str;
            std::cerr << "#makeReply failure " << message << "\n";
            throw std::runtime_error(message);

        } else {
            std::cerr << "#makeReply unsupported reply type " << reply->type << "\n";
            throw std::runtime_error("#makeReply unsupported reply type");
        }
        return result;
    }

    redisAsyncContext* ac;

private:
    int next_id;
    struct request_entry {
        int id;
        RedisConnection* conn;
        Promise<Reply> promise;
    };
    std::map<int, request_entry> pending_requests;
};

void disconnectCallback(const redisAsyncContext* ac, int status) {
    std::cerr << "disconnected :P";
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    uv_loop_t* loop = uv_default_loop();

    RedisConnection conn;
    redisLibuvAttach(conn.ac, loop);
    conn.exec("SET key %s", "value")
        .then([](RedisConnection::Reply r) {
            std::cerr << "#1 ok " << r.str << "\n";
        })
        .error([](std::exception_ptr eptr) {
            std::cerr << "#1 ???\n";
        });
    conn.exec("HGETALL %s", "foo")
        .then([](RedisConnection::Reply r) {
            for(auto& s: r.elements ) {
                std::cerr << "#2 ok " << s.str << "\n";
            }
        })
        .error([](std::exception_ptr eptr) {
            std::cerr << "#2 ???\n";
        });
    
    std::cerr << "uv start :)\n";
    uv_run(loop, UV_RUN_DEFAULT);
    std::cerr << "ev done :)\n";

    return 0;
}