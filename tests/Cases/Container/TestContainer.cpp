#include <CPVFramework/Container/Container.hpp>
#include <CPVFramework/Container/Injector.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	struct Base {
		virtual std::size_t getValue() = 0;
	};

	struct Derived : public Base {
		std::size_t getValue() override { return 1; }
	};

	struct TestSelfInject {
		seastar::shared_ptr<Base> instance;
		TestSelfInject(const seastar::shared_ptr<Base>& instanceVal) :
			instance(instanceVal) { }
		TestSelfInject(const cpv::Container& container) :
			TestSelfInject(container.get<seastar::shared_ptr<Base>>()) { }
	};

	struct TestExternInject {
		seastar::shared_ptr<Base> instance;
		TestExternInject(const seastar::shared_ptr<Base>& instanceVal) :
			instance(instanceVal) { }
	};

	struct TestObjectBase {
		virtual std::size_t getValue() = 0;
	};

	struct TestObjectDerived : public TestObjectBase {
		static void reset() { }
		static void freeResources() { }
		std::size_t getValue() override { return 1; }
	};

	struct TestObjectInject : public TestObjectBase {
		seastar::shared_ptr<Base> instance;
		void reset(const cpv::Container& container) {
			instance = container.get<seastar::shared_ptr<Base>>();
		}
		static void freeResources() { }
		std::size_t getValue() override { return instance->getValue(); }
	};
}

TEST(TestContainer, addInstance) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	auto instanceA = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA->getValue(), 1);
	auto instanceB = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA.get(), instanceB.get());
}

TEST(TestContainer, addFactory) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>>([] (const cpv::Container&) {
		return seastar::make_shared<Derived>();
	});
	auto instanceA = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA->getValue(), 1);
	auto instanceB = container->get<seastar::shared_ptr<Base>>();
	ASSERT_NE(instanceA.get(), instanceB.get());
}

TEST(TestContainer, addTypeDefaultConstruct) {
	auto container = cpv::Container::create();
	container->add<Derived, Derived>();
	auto instance = container->get<Derived>();
	ASSERT_EQ(instance.getValue(), 1);
}

TEST(TestContainer, addTypeContainerConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container->add<TestSelfInject, TestSelfInject>();
	auto instance = container->get<TestSelfInject>();
	ASSERT_EQ(instance.instance->getValue(), 1);
}

TEST(TestContainer, addTypeSharedPtrDefaultConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	auto instance = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instance->getValue(), 1);
}

TEST(TestContainer, addTypeSharedPtrContainerConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container->add<seastar::shared_ptr<TestSelfInject>, seastar::shared_ptr<TestSelfInject>>();
	auto instance = container->get<seastar::shared_ptr<TestSelfInject>>();
	ASSERT_EQ(instance->instance->getValue(), 1);
}

TEST(TestContainer, addTypeLwSharedPtrDefaultConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::lw_shared_ptr<Derived>, seastar::lw_shared_ptr<Derived>>();
	auto instance = container->get<seastar::lw_shared_ptr<Derived>>();
	ASSERT_EQ(instance->getValue(), 1);
}

TEST(TestContainer, addTypeLwSharedPtrContainerConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container->add<seastar::lw_shared_ptr<TestSelfInject>, seastar::lw_shared_ptr<TestSelfInject>>();
	auto instance = container->get<seastar::lw_shared_ptr<TestSelfInject>>();
	ASSERT_EQ(instance->instance->getValue(), 1);
}

TEST(TestContainer, addTypeStdUniquePtrDefaultConstruct) {
	auto container = cpv::Container::create();
	container->add<std::unique_ptr<Derived>, std::unique_ptr<Derived>>();
	auto instance = container->get<std::unique_ptr<Derived>>();
	ASSERT_EQ(instance->getValue(), 1);
}

TEST(TestContainer, addTypeStdUniquePtrContainerConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container->add<std::unique_ptr<TestSelfInject>, std::unique_ptr<TestSelfInject>>();
	auto instance = container->get<std::unique_ptr<TestSelfInject>>();
	ASSERT_EQ(instance->instance->getValue(), 1);
}

TEST(TestContainer, addTypeStdUniquePtrAsSingletonError) {
	auto container = cpv::Container::create();
	container->add<std::unique_ptr<Derived>, std::unique_ptr<Derived>>(cpv::Lifetime::Singleton);
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container->get<std::unique_ptr<Derived>>(),
		"can't create singleton service that's not copy constructible");
}

TEST(TestContainer, addTypeStdSharedPtrDefaultConstruct) {
	auto container = cpv::Container::create();
	container->add<std::shared_ptr<Base>, std::shared_ptr<Derived>>();
	auto instance = container->get<std::shared_ptr<Base>>();
	ASSERT_EQ(instance->getValue(), 1);
}

TEST(TestContainer, addTypeStdSharedPtrContainerConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container->add<std::shared_ptr<TestSelfInject>, std::shared_ptr<TestSelfInject>>();
	auto instance = container->get<std::shared_ptr<TestSelfInject>>();
	ASSERT_EQ(instance->instance->getValue(), 1);
}

TEST(TestContainer, addTypeObjectDefaultConstruct) {
	auto container = cpv::Container::create();
	container->add<cpv::Object<TestObjectBase>, cpv::Object<TestObjectDerived>>();
	auto instance = container->get<cpv::Object<TestObjectBase>>();
	ASSERT_EQ(instance->getValue(), 1);
}

TEST(TestContainer, addTypeObjectContainerConstruct) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container->add<cpv::Object<TestObjectBase>, cpv::Object<TestObjectInject>>();
	auto instance = container->get<cpv::Object<TestObjectBase>>();
	ASSERT_EQ(instance->getValue(), 1);
}

TEST(TestContainer, addTypeExternInject) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container->add<TestExternInject, cpv::Injector<TestExternInject, seastar::shared_ptr<Base>>>();
	auto instance = container->get<TestExternInject>();
	ASSERT_EQ(instance.instance->getValue(), 1);
}

TEST(TestContainer, removeType) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	ASSERT_EQ(container->remove<seastar::shared_ptr<Derived>>().size(), 0);
	ASSERT_EQ(container->remove<seastar::shared_ptr<Base>>().size(), 1);
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container->get<seastar::shared_ptr<Base>>(),
		"no service entry found");
}

TEST(TestContainer, getSingleInstance) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	auto instanceA = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA->getValue(), 1);
	auto instanceB = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA.get(), instanceB.get());
}

TEST(TestContainer, getMultipleInstances) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	auto instances = container->get<std::vector<seastar::shared_ptr<Base>>>();
	ASSERT_EQ(instances.size(), 2);
	ASSERT_EQ(instances.at(0)->getValue(), 1);
	ASSERT_EQ(instances.at(1)->getValue(), 1);
	ASSERT_NE(instances.at(0).get(), instances.at(1).get());
}

TEST(TestContainer, getInstanceEmptyError) {
	auto container = cpv::Container::create();
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container->get<seastar::shared_ptr<Base>>(),
		"no service entry found");
}

TEST(TestContainer, getInstanceMultiError) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container->get<seastar::shared_ptr<Base>>(),
		"more than 1 service entry found");
}

TEST(TestContainer, getInstanceInjectError) {
	auto container = cpv::Container::create();
	container->add<TestSelfInject, TestSelfInject>();
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container->get<TestSelfInject>(),
		"call factory error");
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container->get<TestSelfInject>(),
		"no service entry found");
}

TEST(TestContainer, eachInstances) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	std::size_t total = 0;
	container->each<seastar::shared_ptr<Base>>([&total] (const auto& instance) {
		total += instance->getValue();
	});
	ASSERT_EQ(total, 2);
}

