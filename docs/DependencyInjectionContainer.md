# Dependency injection container

cpv framework provides dependency injection container (Abbreviated to container from now) to let modules integrate with each other by register service to container and resolve service from container. Register service requires service type (interface type), how to construct the  service instance (service factory), and whether to reuse the constructed service instance (service lifetime), resolve service only requires service type.

You may curious what's dependency injection if you don't have prior knowledge, it just simply mean the container can find out what's need for a service construction and pass them to the service constructor, for example, if class `A` has a constructor `A(B b, C c)` then container can resolve instance of `B` and instance of `C` automatically, and pass them to the constructor of `A` to get an instance of `A`, this process is recursive, if class `B` requires class `D` and `E` then container will resolve `D` and `E` for `B` first.

Notice some container in other framework supports member injection instead of constructor injection, but cpv framework's container only supports constructor injection because it's the only correct way to use.

Here is a simple example of container (you may already see it in [Application and modules](./ApplicationAndModules.md)):

``` c++
#include <iostream>
#include <CPVFramework/Container/Container.hpp>

namespace {
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
}

int main() {
	cpv::Container container;

	// register service with it's implmenetation type
	// std::unique_ptr is a wrapper class, cpv framework's container supports several wrapper classes
	container.add<std::unique_ptr<Service>, std::unique_ptr<ServiceImpl>>();

	// resolve service
	auto service = container.get<std::unique_ptr<Service>>();
	service->greet();

	return 0;
}
```

Usually you don't need to construct a container by yourself, [Application](../include/CPVFramework/Application/Application.hpp) will construct it on each cpu core, and you can use the container instance provided by application from `handler` function of module or [HttpContext](../include/CPVFramework/HttpServer/HttpContext.hpp).

## Service lifetime

Service lifetime decide whether container should keep the service instance after resolved and reuse it later, there 3 types of service lifetime supported in cpv framework's container:

- `Transient`: Create new instance every times
	- This doesn't keep the service instance
- `Persistent`: For given container, only create instance once and reuse it in the future (a.k.a Singleton)
	- This keeps the service instance inside the container
	- Service must be copy constructiable or wrapped in shared pointer (like `seastar::shared_ptr`)
- `StoragePersistent`: For given storage, only create instance once and reuse it in the future (a.k.a Scoped)
	- This keeps the service instance inside the stroage given by user when resolving
	- Service must be copy constructiable or wrapped in shared pointer (like `seastar::shared_ptr`)

You can also check [ServiceLifetime](../include/CPVFramework/Container/ServiceLifetime.hpp).

## Register service to container

Let's see how to register service to container, you can also check [Container](../include/CPVFramework/Container/Container.hpp).

### Register service with it's implementation type

The function signature of register service with it's implementation type is:

``` c++
template <class TService, class TImplementation>
void add(ServiceLifetime lifetime = ServiceLifetime::Transient)
```

`TService` should be a base class of `TImplementation`, or `TImplementation` should provide a convert operator for `TService`. `TService` and `TImplementation` can be wrapped in some smart pointer type, here are wrapper types supported by cpv framework's container:

- `std::unique_ptr`
	- Notice you can't use `Persistent` and `StoragePersistent` for service lifetime with this wrapper type.
- `seastar::shared_ptr`
	- A copy of `std::shared_ptr` but not using atomic variables (`std::shared_ptr` is unsupported and should not be used)
- `cpv::Reusable`
	- `std::unique_ptr` with per cpu core free list and custom reset function for fast allocation, like `std::unique_ptr`, you can't use `Persistent` and `StoragePersistent` for service lifetime with this wrapper type.

You should use either of wrapper types if `TService` is a base class of `TImplementation`, otherwise you will face [object slicing](https://en.wikipedia.org/wiki/Object_slicing) problem.

`TImplementation` that requires construct with arguments should provides dependency list (the types of constructor parameters) since C++ doesn't supports constructor reflection yet, there two way to provide dependency list:

- Provide `TImplementation::DependencyTypes` as `std::tuple<types of constructor parameters...>`
- Provide specialized `ServiceDependencyTrait<TImplementation>` with `DependencyTypes` member

In addition, you can inject multiple instances of service that registered multiple times by using `std::vector<TService>` or `cpv::StackAllocatedVector<TService, InitialSize>`, and inject optional instance of service by using `std::optional<TService>` as constructor parameter.

You can also check the definition inside [ServiceTraits](../include/CPVFramework/Container/ServiceTraits.hpp).

Example:

``` c++
#include <iostream>
#include <CPVFramework/Container/Container.hpp>

namespace {
	class Service {
	public:
		virtual void greet() = 0;
		virtual ~Service() = default;
	};

	class Greeter {
	public:
		virtual std::string generate(const std::string& name) const {
			return "Hello! " + name;
		}
		virtual ~Greeter() = default;
	};

	class Name : public std::string {
	public:
		using std::string::string;
	};

	class ServiceImpl : public Service {
	public:
		using DependencyTypes = std::tuple<seastar::shared_ptr<Greeter>, Name>;

		void greet() override {
			std::cout << greeter_->generate(name_) << std::endl;
		}

		ServiceImpl(seastar::shared_ptr<Greeter> greeter, Name name) :
			greeter_(std::move(greeter)), name_(std::move(name)) { }

	private:
		seastar::shared_ptr<Greeter> greeter_;
		Name name_;
	};
}

int main() {
	cpv::Container container;

	// register services that required by ServiceImpl
	container.add<seastar::shared_ptr<Greeter>, seastar::shared_ptr<Greeter>>(
		cpv::ServiceLifetime::Persistent); // reuse greeter instance
	container.add<Name>(Name("john")); // register with instance

	// register service with it's implmenetation type
	// container will construct ServiceImpl with instances of ServiceImpl::DependencyTypes
	container.add<std::unique_ptr<Service>, std::unique_ptr<ServiceImpl>>();

	// resolve service
	auto service = container.get<std::unique_ptr<Service>>();
	service->greet();

	return 0;
}
```

### Register service with it's instance

The function signature of register service with it's instance is:

``` c++
template <class TService>
void add(TService instance);
```

The lifetime of service will be `Persistent` anyway, and please don't register an instance that shared arcross cpu cores unless it's thread safe and lock free.

Example:

``` c++
#include <iostream>
#include <CPVFramework/Container/Container.hpp>

namespace {
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
}

int main() {
	cpv::Container container;

	// register services with it's instance
	// notice the lifetime of service will be Persistent anyway
	container.add<seastar::shared_ptr<Service>>(seastar::make_shared<ServiceImpl>());

	// resolve service
	auto service = container.get<seastar::shared_ptr<Service>>();
	service->greet();

	return 0;
}
```

### Register service with it's factory function

The function signature of register service with it's factory function is:

``` c++
template <class TService, class TFunc,
	std::enable_if_t<std::is_base_of_v<
		ServiceFactoryBase<TService>,
		ServiceFunctionFactory<TService, TFunc>>, int> = 0>
void add(TFunc func, ServiceLifetime lifetime = ServiceLifetime::Transient);
```

`TFunc` should be a function that returns an instance of `TService`, it could takes no arguments, or takes container, or takes container and service storage.

Example:

``` c++
#include <iostream>
#include <CPVFramework/Container/Container.hpp>

namespace {
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
}

int main() {
	cpv::Container container;

	// register services with it's factory function
	// the function could takes cpv::Container and cpv::ServiceStorage
	// for resolving dependencies
	container.add<seastar::shared_ptr<Service>>([] {
		return seastar::make_shared<ServiceImpl>();
	});

	// resolve service
	auto service = container.get<seastar::shared_ptr<Service>>();
	service->greet();

	return 0;
}
```

### Register service with it's factory object

The function signature of register service with it's factory object is:

``` c++
template <class TService>
void add(std::unique_ptr<ServiceFactoryBase<TService>>&& factory,
	ServiceLifetime lifetime = ServiceLifetime::Transient);
```

This overload is rarely used, unless you have a custom templated factory class.

Example:

``` c++
#include <iostream>
#include <CPVFramework/Container/Container.hpp>

namespace {
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

	class ServiceFactory :
		public cpv::ServiceFactoryBase<seastar::shared_ptr<Service>> {
	public:
		seastar::shared_ptr<Service> operator()(
			const cpv::Container& container, cpv::ServiceStorage& storage) const override {
			return seastar::make_shared<ServiceImpl>();
		}
	};
}

int main() {
	cpv::Container container;

	// register services with it's factory object
	container.add<seastar::shared_ptr<Service>>(std::make_unique<ServiceFactory>());

	// resolve service
	auto service = container.get<seastar::shared_ptr<Service>>();
	service->greet();

	return 0;
}
```

## Resolve service from container

Now let's see how to resolve service from container that registered before, you can also check [Container](../include/CPVFramework/Container/Container.hpp).

### Resolve single service

The function signature of resolve single service is:

``` c++
template <class TService>
TService get() const;

template <class TService>
TService get(ServiceStorage& storage) const;
```

The first overload will use the builtin storage inside container, which makes `StoragePersistent` has same effect as `Persistent`, the second overload allows user pass a custom service storage for services registered with `StoragePersistent` lifetime, service instances will store in this storage and reuse when the same storage is given.

`StoragePersistent` is useful for services that want to share same instance for same http request, [HttpContext](../include/CPVFramework/HttpServer/HttpContext.hpp) provides fresh service storage for each http request, and the storage will be destroyed after request finished.

Example:

``` c++
{
	auto service = container.get<seastar::shared_ptr<Service>>();
}
{
	cpv::ServiceStorage storage;
	auto service = container.get<seastar::shared_ptr<Service>>(storage);
}
```

Notice if service not registered or registered multiple times, `Container::get` will throws [ContainerException](../include/CPVFramework/Exceptions/ContainerException.hpp), you can use `Container::getMany` with `std::optional` for optional service as explained next.

### Resolve multiple services

The function signature of resolve multiple services is:

``` c++
template <class T, std::enable_if_t<ServiceTypeTrait<T>::IsCollection, int> = 0>
std::size_t getMany(T& collection) const;

template <class T, std::enable_if_t<ServiceTypeTrait<T>::IsCollection, int> = 0>
std::size_t getMany(T& collection, ServiceStorage& storage) const;
```

Same as resolve single service, the first overload use builtin storage and the second overload takes user given storage. Different to resolve single service, `getMany` will take a reference to the service collection which could be `std::vector<T>` or `cpv::StackAllocatedVector<T, InitialSize>` or `std::optional<T>`, container will resolve all registered `T` into the collection, it could add 0 instance if `T` not registered, or add multiple instances if `T` registered multiple times.

Notice the original content of collection will not be cleared automatically.

`std::optional<T>` is a specialization for getting optional service, it will be untouched if service not registered, or set to the instance of service registered, or set to the lastest instance of service if service registered multiple times.

You can make the container supports more collection type by specialize `ServiceTypeTrait`, see [ServiceTraits](../include/CPVFramework/Container/ServiceTraits.hpp) for more information.

Example:

``` c++
{
	std::vector<seastar::shared_ptr<Service>> services;
	container.getMany(services);
}
{
	cpv::ServiceStorage storage;
	std::optional<seastar::shared_ptr<Service>> optionalService;
	container.getMany(optionalService, storage);
}
```

