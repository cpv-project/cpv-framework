# Http server request handlers

The http server provided by cpv framework uses handlers (middlewares) to handle http request, the interface of handlers is [HttpServerRequestHandlerBase](../include/CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp), it takes two arguments, first argument is [HttpContext](../include/HttpServer/HttpContext.hpp) (contains request, response, and container), second argument is the next handler, the handler can decide either handle the http request or pass it to the next handler.

Here is an example about how to implement custom handler:

``` c++
#include <seastar/core/app-template.hh>
#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Application/Modules/LoggingModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerModule.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>

namespace {
	using namespace cpv;

	class MyHandler : public HttpServerRequestHandlerBase {
		seastar::future<> handle(
			HttpContext& context,
			const HttpServerRequestHandlerIterator& next) const override {
			HttpRequest& request = context.getRequest();
			if (request.getUrl() != "/hello") {
				// if url not "/hello" then let next handler to handle it
				return (*next)->handle(context, next + 1);
			}
			HttpResponse& response = context.getResponse();
			return extensions::reply(response, "Hello!");
		}
	};
}

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		cpv::Application application;
		application.add<cpv::LoggingModule>();
		application.add<cpv::HttpServerModule>([] (auto& module) {
			module.addCustomHandler(seastar::make_shared<MyHandler>());
		});
		return application.runForever();
	});
	return 0;
}
```

Compile and run it, then access `http://localhost:8000/hello` and you will see `Hello!`, while access other path will given `Not Found`.

## HttpContext, HttpRequest and HttpResponse

As the examples shows, you can use `getRequest` to get the [HttpRequest](../include/CPVFramework/Http/HttpRequest.hpp) and `getResponse` to get the [HttpResponse](../include/CPVFramework/Http/HttpResponse.hpp) from `HttpContext`, these two classes is designed for generic purpose which can be used in both http server and http client, they also provide stream api to reading or writing the body, the body of `HttpRequest` is [InputStreamBase](../include/CPVFramework/Stream/InputStreamBase.hpp), and the body of `HttpResponse` is [OutputStreamBase](../include/CPVFramework/Stream/OutputStreamBase.hpp).

For convenient, cpv framework provides extensions functions for `InputStreamBase` and `OutputStreamBase`, please check [InputStreamExtensions](../include/CPVFramework/Stream/InputStreamExtensions.hpp) and [OutputStreamExtensions](../include/CPVFramework/Stream/OutputStreamExtensions.hpp).

There also extensions for `HttpRequest` and `HttpResponse`, the one of the most useful extensions function is `extensions::reply` for `HttpResponse`, please check [HttpRequestExtensions](../include/CPVFramework/Http/HttpRequestExtensions.hpp) and [HttpResponseExtensions](../include/CPVFramework/Http/HttpResponseExtensions.hpp).

Sometimes the handler may want to access the container provided by application to resolve some required services, `HttpContext` provides `getService` and `getManyServices` corresponding to `Container::get` and `Container::getMany`. The reason about `HttpContext` doesn't provide direct access to `Container` is `getService` and `getManyServices` will use the service storage managed by `HttpContext`, which allow services registered with `StoragePersistent` lifetime shared for same http request but destroyed after request finished.

## Packet (scattered message)

Seastar framework allow user to construct and send scattered message (discontinuous data fragments in memory) via a socket, it can avoid unnecessary allocation and memory copy to improve performance for large messages. To construct scattered message in seastar framework, you can use `seastar::scattered_message` (which is a wrapper of `seastar::packet`) or `seastar::packet` (which is a wapper of posix's `iovec`).

In cpv framework, you should use [Packet](../include/CPVFramework/Utility/Packet.hpp) instead of `seastar::scattered_message` or `seastar::packet`, `Packet` is an union of `seastar::temporary_buffer` and `seastar::packet`, it can use `seastar::temporary_buffer` for single fragment (which is zero allocation) to avoid the minimal allocation caused by `seastar::packet`.

The example of `Packet`:

``` c++
cpv::Packet p;

p.append("abc"); // it's single fragment (seastar::temporary_buffer) now
p.append("def"); // it's multiple fragments (seastar::packet) now
p.append(" value: "); // append to multiple fragments
p.append(cpv::SharedString::fromInt(123)); // append to multiple fragments

if (auto ptr = packet.getIfSingle()) {
	// release as seastar::temporary_buffer if packet only contains 1 fragment
	seastar::temporary_buffer<char> buf = ptr->release();
} else if (auto ptr = packet.getIfMultiple()) {
	// release as seastar::net::packet if packet contains multiple fragments
	seastar::net::packet p_ = ptr->release();
} else {
	// packet contains 0 fragment
}
```

You may already notice that `OutputStreamBase` takes `Packet` for `write` function, so you could just pass it to the stream instead of release the underlying type manually. In addition, you can use the `<<` operator to write the packet to `seastar::data_sink` for original seastar sockets.

## Built-in request handlers

cpv framework provides some built-in request handlers, here is the list of handlers and their simple introduction.

### [HttpServerRequest404Handler](../include/CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp)

This is a handler returns "Not Found" as plain text with 404 status code.

It should be the last handler in the handler list, [HttpServerModule](../include/CPVFramework/Application/Modules/HttpServerModule.hpp) will register this handler by default at `RegisterTailServices` application state which put it after handlers registered in previous states. You can also set a custom 404 handler by using `HttpServerModule::set404Handler`.

### [HttpServerRequest500Handler](../include/CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp)

This is a handler always pass request to next handler and capture for exceptions, if exception occurs, this handler will generate an uuid, log exception details to logger with the uuid, and return the uuid and the common error message with 500 status code, the uuid is useful for finding the detail corresponding to the error page displayed in the brower.

It should be the first handler in the handler list, [HttpServerModule](../include/CPVFramework/Application/Modules/HttpServerModule.hpp) will register this handler by default at `RegisterHeadServices` application state which put it before handlers registered in next states. You can also set a custom 500 handler by using `HttpServerModule::set500Handler`.

### [HttpServerRequestFunctionHandler](../include/CPVFramework/HttpServer/Handlers/HttpServerRequestFunctionHandler.hpp)

This is a handler that takes a custom function object (lambda function object) and use it to handler requests, the signature of function is:

``` c++
seastar::future<> func(HttpContext& context);
```

Notice it doesn't takes the next handler, so use it alone is not practical, usually it should be used with `HttpServerRequestRoutingHandler` for handling a given route.

### [HttpServerRequestParametersFunctionHandler](../include/CPVFramework/HttpServer/Handlers/HttpServerRequestParametersFunctionHandler.hpp)

This is a handler that takes a custom function object (lambda function object) and use it to handler requests, different to `HttpServerRequestFunctionHandler`, this handler also takes a tuple that defines how to retrive extra parameters from request, for example:

``` c++
using namespace cpv::extensions::http_context_parameters;

auto handler = seastar::make_shared<HttpServerRequestParametersFunctionHandler>(
	std::make_tuple(PathFragment(1), Query("key")),
	[] (HttpContext& context, cpv::SharedString id, cpv::SharedString key) {
		return extensions::reply(
			context.getResponse(),
			cpv::Packet().append(id).append(key));
	});

// call handler->handle(context, next + 1) will invokes:
// func(context,
//     extensions::getParameter(context, PathFragment(1)),
//     extensions::getParameter(context, Query("key")));
//
// which equals to
// func(context,
//     context.getRequest().getUri().getPathFragment(1),
//     context.getRequest().getUri().getQueryParameter("key"));
//
// which equals to
// func(context, "123", "456");
// when url is "/get/123?key=456"
```

You can make it support more types by providing overload of `ParameterType cpv::getParameter(HttpContext, TupleElementType)`.

Like `HttpServerRequestFunctionHandler`, the function doesn't takes the next handler, so use it alone is not practical, usually it should be used with `HttpServerRequestRoutingHandler` for handling a given route.

### [HttpServerRequestRoutingHandler](../include/CPVFramework/HttpServer/Handlers/HttpServerRequestRoutingHandler.hpp)

This is a handler that manages a routing map and decide use which handler to handle the request by the path of the url, you can use `route` to associate a handler to given path (can be full path or wildcard path), `removeRoute` to disassociate the handler for given path, and `getRoute` to get the handler associated with given path.

For more information about the `route` function and the format of path, please see the document of `HttpServerRoutingModule` in [application and modules](./ApplicationAndModules.md), the `route` function of `HttpServerRoutingModule` is just a forwarder to `route` function of `HttpServerRequestRoutingHandler`.

### [HttpServerRequestStaticFileHandler](../include/CPVFramework/HttpServer/Handlers/HttpServerRequestStaticFileHandler.hpp)

This is a handler that returns content of files, it map files under `pathBase` parameter to `urlBase` parameter, for example if `pathBase` is `./static` and `urlBase` is `/static`, `/static/1.txt` will map to `./static/1.txt`. It will invoke the next handler if file not exists.

It contains following features:

- Supports pre compressed gzip files, for example if `./static/1.txt.gz` exists it will be used instead of `./static/1.txt`
- Supports bytes range (the `Range` header)
- Supports return 304 not modified when `If-Modified-Since` matched
- Supports lru memory cache for file content (by default it cache 16 files that not greater than 1MB in memory)

For example please see the document of `HttpServerRoutingModule` in [application and modules](./ApplicationAndModules.md), the `routeStaticFile` function of `HttpServerRoutingModule` will construct `HttpServerRequestStaticFileHandler` and register to `HttpServerRequestRoutingHandler`.

