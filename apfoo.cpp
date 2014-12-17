#include "json_mo_writer.h"
#include "json_mo_parser.h"

#include <tinfra/tstring.h>
#include <tinfra/tcp_socket.h>
#include <tinfra/memory_stream.h>
#include <tinfra/mo.h>
#include <tinfra/fmt.h>
#include <tinfra/logger.h>
#include <tinfra/stream.h>

#include <memory> // for unique_ptr
#include <vector>
//
// deploy_app
//
struct deployment_request {
    std::string uri;
    std::string implementation;

    TINFRA_MO_MANIFEST(deployment_request) {
        TINFRA_MO_FIELD(uri);
        TINFRA_MO_FIELD(implementation);
    }
};

TINFRA_MO_IS_RECORD(deployment_request);

struct deploymnent_response {
    int status;
    std::string message;

    deploymnent_response():
        status(0),
        message("")
    {
    }
    TINFRA_MO_MANIFEST(deploymnent_response) {
        TINFRA_MO_FIELD(status);
        TINFRA_MO_FIELD(message);
    }
};

TINFRA_MO_IS_RECORD(deploymnent_response);

void deploy_app(deployment_request const& params, deploymnent_response& resp)
{
    tinfra::log_info("deploy_app called");
    resp.message = tinfra::tsprintf("deployed %s under %s", params.implementation, params.uri);
}

//
// http_server
//

using tinfra::tstring;
using tinfra::input_stream;
using tinfra::output_stream;

struct http_request {
    tstring request_uri;
    std::string content;

    http_request(input_stream& in):
        in(in)
    {
    }
    void parse_headers();

private:
    input_stream& in;
};

struct http_response {
    int     status_code;
    tstring status_text;

    std::string content_buffer;
    tinfra::memory_output_stream content_stream;
    
    bool content_sent;
    bool headers_sent;
    
    http_response(output_stream& out):
        status_code(200),
        status_text("OK"),
        content_buffer(""),
        content_stream(content_buffer),
        content_sent(false),
        headers_sent(false),
        out(out)
    {
    }

    void set_error_response(int code, tstring text) {
        status_code = code;
        status_text = text;
    }

    tinfra::output_stream& get_output() 
    {
        return content_stream;
    }

    void send_headers()
    {
        tprintf(this->out, "HTTP/1.1 %i %s\r\n", this->status_code, this->status_text);
        tprintf(this->out, "Connection: close\r\n");
        tprintf(this->out, "Content-length: %i\r\n", this->content_buffer.size());
                // IDEA, tprintf is cool, but does parsing in runtime
                // create
                // tout(this,out, "Content-length", i, "\r\n"); which will have same
                // semantics, minimal code and resolve sizing/parsing problem at runtime
        tprintf(this->out, "\r\n");
        headers_sent =  true;
    }

    void send() {
        if( ! headers_sent ) {
            send_headers();
            headers_sent =  true;
        }
        if( ! content_sent ) {
            tinfra::write_all(this->out, this->content_buffer);
            content_sent = true;
        }
    }
private:
    output_stream& out;
};

typedef std::function<void(http_request&, http_response&)> http_handler;

class http_server {
public:
    http_server(tinfra::tstring name, int port):
        server_socket(new tinfra::tcp_server_socket(name, port))
    {
    }
    void add_handler(tstring uri_pattern, http_handler handler)
    {
        this->map_entries.push_back( map_entry {uri_pattern, handler } );
    }
    void run();
private:
    struct map_entry {
        std::string  uri;
        http_handler  handler;
    };

    http_handler* find_handler(http_request const& request);

    bool shall_continue() {
        return true;
    }
    std::vector<map_entry> map_entries;
    std::unique_ptr<tinfra::tcp_server_socket> server_socket;
};

void http_server::run() {
    using tinfra::tcp_client_socket;
    using tinfra::tsprintf;
    using tinfra::log_error;
    using tinfra::log_info;

    while( shall_continue() ) {
        try {
            std::string peer_address;
            std::auto_ptr<tcp_client_socket> client_socket(server_socket->accept(peer_address));
            log_info(tsprintf("connection from: %s", peer_address));
            http_request req(*client_socket);
            http_response resp(*client_socket);
            try {
                req.parse_headers();
            } catch( std::exception& e ) {
                log_error(tsprintf("error, failed to parse headers: %s", e.what()));
                resp.set_error_response(400, "bad request");
                resp.send();
                client_socket->close();
                continue;
            }

            log_info(tsprintf("request_uri: %s", req.request_uri));

            http_handler* handler = find_handler(req);

            if( ! handler ) {
                resp.set_error_response(404, "not found");
                resp.send();
                log_error(tsprintf("error, not found: %s", req.request_uri));
                client_socket->close();
                continue;
            }

            try {
                (*handler) (req, resp);
            } catch( std::exception& e ) {
                resp.set_error_response(500, "internal server error");
                log_error(tsprintf("error, during running for %s: %s", req.request_uri, e.what()));
                resp.send();
                client_socket->close();
                continue;
            }
            resp.send();
        } catch( std::exception& e ) {
            log_error(tsprintf("error, during processing request ...: %s", e.what()));
        }
    }
}

http_handler* http_server::find_handler(http_request const& request)
{
    for(map_entry& me: this->map_entries ) {
        if( me.uri == request.request_uri) {
            return & (me.handler );
        }
    }
    return 0;
}

void http_request::parse_headers()
{
    this->request_uri = "/deploy_app";
    this->content = "{ \"uri\": \"/foo\", \"implementation\": \"foo.cpp\" } ";
}

//
// json
//

template <typename RequestType>
RequestType json_parse_request(http_request& http_req)
{
    const std::string content = http_req.content;
    tinfra::memory_input_stream mis(content);
    tinfra::json_lexer lexer(mis);
    json_mo_parser json_mo_parser(lexer);
    RequestType req_value;
    tinfra::mutate("response", req_value, json_mo_parser);
    return req_value;
}
template <typename ResponseType>
void json_render_response(http_response& http_resp, ResponseType& resp_value)
{
    json_mo_writer json_writer(http_resp.get_output());
    //tinfra::mo_process(resp_value, json_writer);
    tinfra::process("response", resp_value, json_writer);
}


template <typename RequestType, typename ResponseType, typename FUN>
http_handler json_transform_func(FUN f)
{
    return [=] (http_request& req, http_response& resp) {
        const RequestType  request_value = json_parse_request<RequestType>(req);
        ResponseType response_value;
        f(request_value, response_value);
        json_render_response<ResponseType>(resp, response_value);
    };
}

int main(int argc, char** argv)
{
    tinfra::public_tracer::process_params(argc, argv);

    http_server server("localhost", 8080);
    server.add_handler("/deploy_app", json_transform_func<deployment_request, deploymnent_response>(deploy_app));

    server.run();
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
