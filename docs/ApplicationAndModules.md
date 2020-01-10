# Application and modules

cpv framework is a modular framework, there 3 core types used to build a cpv application, you will learn them in this chapter:

- [Application](../include/CPVFramework/Application/Application.hpp): used to manage modules and the container
- [ModuleBase](../include/CPVFramework/Application/ModuleBase.hpp): the interface of module
- [Container](../include/CPVFramework/Container/Container.hpp): the dependency injection container, modules will use it to integrate with each other

but first, let's see how to create an empty application.

## Create an empty seastar application

Since cpv framework is based in seastar framework, let's see how to create an empty seastar application:

``` c++
#include <seastar/core/app-template.hh>
#include <seastar/core/future.hh>

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		return seastar::make_ready_future<>();
	});
	return 0;
}
```

Compile it with (if you didn't install seastar framework and cpv framework, see project README):

``` sh
g++-9 $(pkg-config --cflags seastar) \
	empty-seastar-app.cpp \
	$(pkg-config --libs seastar)
```

First, we created a `seastar::app_template` instance which represents a seastar application. Then we pass the command line arguments and a lambda function that returns a future object to `app.run`. `app.run` will parse the command line arguments, setup the reactor and the task schduler for asynchronous operations, and run the function to get a initial future object (in this empty application it's a future object resolved at the beginning), then blocking until the initial future object resolved, reactor and task schduler will keep working while `app.run` is blocking. Finally application exit with status code 0.

If you want to sleep 1 second before application exit, you can write:

``` c++
#include <seastar/core/app-template.hh>
#include <seastar/core/future.hh>
#include <seastar/core/sleep.hh>

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		return seastar::sleep(std::chrono::seconds(1));
	});
	return 0;
}
```

The function passed to `app.run` will return a future object that resolved after 1 second, notice the function itself will return immediately, it will create an asynchronous timer that execute after 1 second and resolve the future object it returned.

It's strongly recommended to read the tutorial of seastar framework if you don't familiar with it:

- https://github.com/scylladb/seastar/blob/master/doc/mini-tutorial.md
- https://github.com/scylladb/seastar/blob/master/doc/tutorial.md

## Create an empty cpv application

Now, let's see how to create an empty cpv application:

``` c++
#include <seastar/core/app-template.hh>
#include <CPVFramework/Application/Application.hpp>

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		cpv::Application application;
		return application.runForever();
	});
	return 0;
}
```

Compile it with:

``` sh
g++-9 $(pkg-config --cflags seastar) \
	$(pkg-config --cflags cpvframework) \
	empty-cpv-app.cpp \
	$(pkg-config --libs seastar) \
	$(pkg-config --libs cpvframework)
```

Run it and it will do nothing but blocking until you press Ctrl+C.

## Add a simple custom module

If we want this cpv application do something meaningful, we need to add modules, to explain how modules work, let's add a simple custom module that prints message when application start and stop:

``` c++
#include <seastar/core/app-template.hh>
#include <seastar/core/future.hh>
#include <CPVFramework/Application/Application.hpp>

namespace {
	using namespace cpv;

	class MyModule : public ModuleBase {
		seastar::future<> handle(Container& container, ApplicationState state) override {
			if (state == ApplicationState::Starting) {
				std::cout << "application is starting" << std::endl;
			} else if (state == ApplicationState::Stopping) {
				std::cout << "application is stopping" << std::endl;
			}
			return seastar::make_ready_future<>();
		}
	};
}

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		cpv::Application application;
		application.add<MyModule>();
		return application.runForever();
	});
	return 0;
}
```

In this example, we create a custom class that implements the module interface `ModuleBase`, the module interface only has one function `handle` and it will be invoked at each state of application like a callback. (we will learn about the container later) Notice the `handle` function returns future object, that means it can do asynchronous processing, in this case we don't need asynchronous processing so just return a ready future object.

After compile and run it, you will see output like this:

``` text
application is starting
application is starting
application is starting
application is starting
```

and after you press Ctrl+C, you will see output like this:

``` text
application is stopping
application is stopping
application is stopping
application is stopping
```

You might wonder why the message printed multiple times, that's because cpv application will create and execute modules on each cpu cores separately, that makes application code thread safe thus we don't need thread lock or atomic variables unless we want to access shared non thread local static variables.

Notice the order of modules is matter, it will affect the execution order of `handle` and the position of services inside dependency injection container if the service will be registered multiple times.

For more description about application states and how order of modules is matter, please check [Application.hpp](../include/CPVFramework/Application/Application.hpp) and [ApplicationState.hpp](../include/CPVFramework/Application/ApplicationState.hpp).

## Register and resolve services with dependency injection container

A cpv application can add multiple modules, and modules integrate with each other with a dependency injection container, if you don't have any knowledge about dependency injection container, just see it as a dictionary that maps types to instance factories. cpv application will create dependency injection container (Abbreviated to container from now) on each cpu core like modules, and all modules on the same cpu core can share the same container instance.

Here is an example shows how to register and resolve service in modules:

``` c++
#include <seastar/core/app-template.hh>
#include <seastar/core/future.hh>
#include <CPVFramework/Application/Application.hpp>

namespace {
	using namespace cpv;

	class Service {
	public:
		virtual void greet() = 0;
		virtual ~Service() = default;
	};

	class ServiceImpl : public Service {
	public:
		void greet() override {
			std::cout << "Hello from ServiceImpl" << std::endl;
		}
	};

	class AddServiceModule : public ModuleBase {
		seastar::future<> handle(Container& container, ApplicationState state) override {
			if (state == ApplicationState::RegisterServices) {
				container.add<std::unique_ptr<Service>, std::unique_ptr<ServiceImpl>>();
			}
			return seastar::make_ready_future<>();
		}
	};

	class UseServiceModule : public ModuleBase {
		seastar::future<> handle(Container& container, ApplicationState state) override {
			if (state == ApplicationState::Starting) {
				auto service = container.get<std::unique_ptr<Service>>();
				service->greet();
			}
			return seastar::make_ready_future<>();
		}
	};
}

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		cpv::Application application;
		application.add<AddServiceModule>();
		application.add<UseServiceModule>();
		return application.runForever();
	});
	return 0;
}
```

In this example, `AddServiceModule` register a service with it's implmenetation type to container, and `UseServiceModule` get a service instance and invoke member function of it, after compile and run it you will see output like:

``` text
Hello from ServiceImpl
Hello from ServiceImpl
Hello from ServiceImpl
Hello from ServiceImpl
```

Please also check [the document about dependency injection container](./DependencyInjectionContainer.md) for advance usage such as register service with instance or custom factory, specify lifetime, define dependencies that will inject to constructor.

## Add module with custom initialize function

Sometime you might want to customize configuration provided by module, `Application::add` has an overload that takes a custom initialize function for module, here is an example shows how to take argument from command line and pass it to module:

``` c++
#include <seastar/core/app-template.hh>
#include <seastar/core/future.hh>
#include <CPVFramework/Application/Application.hpp>

namespace {
	using namespace cpv;

	class MyModule : public ModuleBase {
	public:
		seastar::future<> handle(Container& container, ApplicationState state) override {
			if (state == ApplicationState::Starting) {
				std::cout << "Hello! " << name_ << std::endl;
			} else if (state == ApplicationState::Stopping) {
				std::cout << "Bye! " << name_ << std::endl;
			}
			return seastar::make_ready_future<>();
		}

		void setName(std::string name) {
			name_ = std::move(name);
		}

	private:
		std::string name_;
	};
}

int main(int argc, char** argv) {
	seastar::app_template app;
	app.add_options()("name", boost::program_options::value<std::string>(), "your name");
	app.run(argc, argv, [&app] {
		cpv::Application application;
		application.add<MyModule>([&app] (auto& module) {
			auto& config = app.configuration();
			auto& nameOption = config["name"];
			if (!nameOption.empty()) {
				module.setName(nameOption.as<std::string>());
			}
		});
		return application.runForever();
	});
	return 0;
}
```

Compile and run it with `./a.out --name john` and you will see one or more `Hello! john` when starting application and `Bye! john` when stopping application. Notice the custom initialize function will run on each cpu core because module will be created on each cpu core too.

Also notice that custom initialize function will be executed at the `CallingCustomIntializeFunctions` application state, and it's after `runForever` executed, so don't capture reference of variables on the stack that defines `cpv::Application application`.

## Built-in modules provided by cpv framework

Although you can define custom modules and do anything you want, you will likely to use built-in modules provided by cpv framework to build a web application (you can write your own http server module or routing module if you really want), here is the modue list and their simple introduction, you can found all built-in modules inside [Modules](../include/CPVFramework/Application/Modules) folder.

### [LoggingModule](../include/CPVFramework/Application/Modules/LoggingModule.hpp)

This is a moudle used to provide logger instance (type is `seastar::shared_ptr<cpv::Logger>`), example:

``` c++
application.add<cpv::LoggingModule>();
```

By default, it uses console logger with Notice level, you can set custom logger by using custom initialize function:

``` c++
application.add<cpv::LoggingModule>(auto& module) {
	module.setLogger(cpv::Logger::createConsole(cpv::LogLevel::Info));
});
```

And you can get the logger instance from the container:

``` c++
auto logger = container.get<seastar::shared_ptr<cpv::Logger>>();
```

### [HttpServerModule](../include/CPVFramework/Application/Modules/HttpServerModule.hpp)

This is a module used to configure, start, and stop http server, example:

``` c++
application.add<cpv::HttpServerModule>([] (auto& module) {
	module.getConfig().setListenAddresses({ "0.0.0.0:8000", "127.0.0.1:8001" });
});
```

The http server provided by cpv framework uses handlers (middlewares) to handle http request, the interface of handlers is [HttpServerRequestHandlerBase](../include/CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp), it takes two arguments, first argument is [HttpContext](../include/HttpServer/HttpContext.hpp) (contains request, response, and container), second argument is the next handler, the handler can decide either handle the http request or pass it to the next handler.

This module by default register a 500 handler at the begin of hander list, and a 404 handler at the last of handler list, the 500 handler invoke the next handler and reply 500 internal server error when exception occurs, the 404 handler just reply 404 not found because it should be the last handler and previous handlers didn't handle the http request.

You can replace custom 500 handler and 404 handler by using custom initialize function:

``` c++
application.add<cpv::HttpServerModule>(auto& module) {
	module.set404Handler([] (cpv::HttpContext& context) {
		// redirect to /404 if path not found
		return cpv::extensions::redirectTo(context.getResponse(), "/404");
	});
	// unlike 400 handler, 500 handler should only act when exception occurs,
	// you could see src/HttpServer/Handlers/HttpServerRequest500Handler.cpp
	// if you want to write a custom 500 handler.
	module.set500Handler(seastar::make_shared<My500Handler>());
});
```

And you can add a custom handler between 500 handler and 404 handler (it's recommended to use HttpServerRoutingModule for most cases):

``` c++
application.add<cpv::HttpServerModule>(auto& module) {
	module.addCustomHandler(seastar::make_shared<MyHandler>());
});
```

For more information please check the document about [http server request handlers](./HttpServerRequestHandlers.md).

### [HttpServerRoutingModule](../include/CPVFramework/Application/Modules/HttpServerRoutingModule.hpp)

This is a module used to adding request routing handler for http server, you can bind custom handlers for given method and path, for example:

``` c++
application.add<cpv::HttpServerRoutingModule>(auto& module) {
	using namespace cpv::extensions::http_context_parameters;

	module.route(cpv::constants::GET, "/", [] (cpv::HttpContext& context) {
		return cpv::extensions::reply(context.getResponse(), "index page");
	});

	module.route(cpv::constants::GET, "/get/*/detail",
		std::make_tuple(PathFragment(1)),
		[] (cpv::HttpContext& context, cpv::SharedString id) {
			return cpv::extensions::reply(context.getResponse(), id);
		});

	module.route(cpv::constants::GET, "/get-more/**",
		[] (cpv::HttpContext& context) {
			return cpv::extensions::reply(
				context.getResponse(),
				context.getRequest().getUrl());
		});

	module.routeStaticFile("/static", "./static");
});
```

##### route

The `route` function has these overloads:

- `route(method, path, handler)`:
	- the type of `handler` is `seastar::shared_ptr<cpv::HttpServerRequestHandlerBase>`
- `route(method, path, func)`:
	- `func` can be a lambda function that takes `cpv::HttpContext` and returns `seastar::future<>`
- `route(method, path, params, func)`:
	- `params` is a tuple controls what parameters will be pass to `func`, for example
		- `PathFragment(1)` => `context.getRequest().getUri().getPathFragment(1)`
		- `Query("key")` => `context.getRequest().getUri().getQueryParameter("key")`
		- `Service<MyService>()` => `context.getService<MyService>()`
		- `JsonModel<MyModel>()` => `extensions::readBodyStreamAsJson<MyModel>(context.getRequest())`
		- `FormModel<MyModel>()` => `extensions::readBodyStreamAsForm<MyModel>(context.getRequest())`
		- you can make it support more types by provide `cpv::extensions::getParameter(context, yourType)`
	- `func` can be a lambda function that takes `cpv::HttpContext` and parameters

In these overloads, `method` means http request method like `"GET"` (same as `cpv::constants::GET`) or `"POST"` (same as `cpv::constants::POST`); `path` means the path of url part, path may contains `*` or `**` as path fragment (parts between `/`), `*` represents one arbitrary path fragment, `**` represents one or more arbitrary path fragments, `**` can only be used at the end of the path.

For example, `/get/*/detail` matches `/get/123/detail` and `/get/321/detail` but not `/get/123/some/detail`, `/get-more/**` matches `/get-more/a` and `/get-more/a/b` but not `/get-more` (`**` not represents zero path fragments).

##### routeStaticFile

The parameters of `routeStaticFile` is:

```
routeStaticFile(urlBase, pathBase, cacheControl="", maxCacheFileEntities=16, maxCacheFileSize=1048576)
```

The `cacheControl` parameter is for "Cache-Control" header, for example you can set it to "max-age=84600, public".

The `maxCacheFileEntities` parameter controls how many files can be cache in memory to improve performance, you can set it to 0 to disable caching.

The `maxCacheFileSize` parameter controls the maximum size (in bytes) of file that able to cache in memory.

The static file handler supports pre compressed gzip files, for example if urlBase is `/static` and pathBase is `./static`, when client request `/static/1.txt`, the handler will search `./static/1.txt.gz` before `./static/1.txt` and return file contents if either of them exists. You can generate pre compressed gzip files by using tool `make-gzip.sh` under `tools` folder, just cd to static folder and execute the tool.

