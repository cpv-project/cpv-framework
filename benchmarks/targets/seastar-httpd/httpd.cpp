#include <seastar/core/prometheus.hh>
#include <seastar/http/httpd.hh>
#include <seastar/http/handlers.hh>
#include <seastar/http/function_handlers.hh>
#include <seastar/http/file_handler.hh>
#include <seastar/http/api_docs.hh>

namespace bpo = boost::program_options;

using namespace seastar;
using namespace httpd;

class handl : public httpd::handler_base {
public:
    virtual future<std::unique_ptr<reply> > handle(const sstring& path,
            std::unique_ptr<request> req, std::unique_ptr<reply> rep) {
        rep->_content = "hello";
        rep->done("html");
        return make_ready_future<std::unique_ptr<reply>>(std::move(rep));
    }
};

void set_routes(routes& r) {
    function_handler* h1 = new function_handler([](const_req req) {
        return "Hello World!";
    });
    r.add(operation_type::GET, url("/"), h1);
    r.add(operation_type::GET, url("/file").remainder("path"),
            new directory_handler("/"));
}

int main(int ac, char** av) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(8000),
            "HTTP Server port");
    return app.run_deprecated(ac, av, [&] {
        auto&& config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        auto server = new http_server_control();
        auto rb = make_shared<api_registry_builder>("apps/httpd/");
        return server->start().then([server] {
            return server->set_routes(set_routes);
        }).then([server, rb]{
            return server->set_routes([rb](routes& r){rb->set_api_doc(r);});
        }).then([server, rb]{
            return server->set_routes([rb](routes& r) {rb->register_function(r, "demo", "hello world application");});
        }).then([server, rb] {
            prometheus::config ctx;
            return prometheus::start(*server, ctx);
        }).then([server, port] {
            seastar::listen_options lo;
            lo.reuse_address = true;
            lo.listen_backlog = 65535;
            return server->listen(port, lo);
        }).then([server, port] {
            std::cout << "Seastar HTTP server listening on port " << port << " ...\n";
            engine().at_exit([server] {
                return server->stop();
            });
        });

    });
}
