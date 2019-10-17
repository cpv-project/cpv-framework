# Application and modules

cpv framework is a modular framework, there 3 core types used to build a cpv application, you will learn them in this chapter:

- `cpv::Application`: used to manage modules and the container
- `cpv::ModuleBase`: the interface of module
- `cpv::Container`: the dependency injection container, modules will use it to integrate with each other

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

In this example, we create a custom class that implements the module interface `cpv::ModuleBase`, the module interface only has one function `handle` and it will be invoked at each state of application like a callback. (we will learn about the container later) Notice the `handle` function returns future object, that means it can do asynchronous processing, in this case we don't need asynchronous processing so just return a ready future object.

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

For more description about application states, please check [Application.hpp](../include/CPVFramework/Application/Application.hpp) and [ApplicationState.hpp](../include/CPVFramework/Application/ApplicationState.hpp).

## Register and resolve services with dependency injection container

A cpv application can add multiple modules, and modules integrate with each other with a dependency injection container, if you don't have any knowledge about dependency injection container, just see it as a dictionary that maps types to instance factories. cpv application will create dependency injection container (Abbreviated to container from now) on each cpu core like modules, and all modules on the same cpu core can share the same container instance.

Here is an example shows how to register and resolve service in modules:

``` c++
#include <memory>
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

Please also check [the document about dependency injection container](./DependencyInjectionContainer.md) for advance usages such as register service with instance or custom factory, specify lifetime, define dependencies that will inject to constructor.

## Add module with custom initialize function

TODO

## Builtin modules provided by cpv framework

Although you can define custom modules and do anything you want, you will likely to use builtin modules provided by cpv framework to build a web application (you can write your own http server module or routing module if you really want), here is the modue list and their simple introduction, you can found all builtin modules inside [Modules](../include/CPVFramework/Application/Modules) folder.

### LoggingModule

TODO

### HttpServerModule

TODO

### HttpServerRoutingModule

TODO

