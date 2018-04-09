#include <CPVFramework/Container/Container.hpp>
#include <CPVFramework/Container/Injector.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	struct Base {
		virtual std::size_t getValue() = 0;
	};

	struct Derived : public Base {
		virtual std::size_t getValue() override { return 1; }
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
}

TEST(TestContainer, addInstance) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	auto instanceA = container.get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA->getValue(), 1);
	auto instanceB = container.get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA.get(), instanceB.get());
}

TEST(TestContainer, addFactory) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>>([] (const cpv::Container&) {
		return seastar::make_shared<Derived>();
	});
	auto instanceA = container.get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA->getValue(), 1);
	auto instanceB = container.get<seastar::shared_ptr<Base>>();
	ASSERT_NE(instanceA.get(), instanceB.get());
}

TEST(TestContainer, addTypeDefaultConstruct) {
	cpv::Container container;
	container.add<Derived, Derived>();
	auto instance = container.get<Derived>();
	ASSERT_EQ(instance.getValue(), 1);
}

TEST(TestContainer, addTypeContainerConstruct) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container.add<TestSelfInject, TestSelfInject>();
	auto instance = container.get<TestSelfInject>();
	ASSERT_EQ(instance.instance->getValue(), 1);
}

TEST(TestContainer, addTypeSharedPtrDefaultConstruct) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	auto instance = container.get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instance->getValue(), 1);
}

TEST(TestContainer, addTypeSharedPtrContainerConstruct) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container.add<seastar::shared_ptr<TestSelfInject>, seastar::shared_ptr<TestSelfInject>>();
	auto instance = container.get<seastar::shared_ptr<TestSelfInject>>();
	ASSERT_EQ(instance->instance->getValue(), 1);
}

TEST(TestContainer, addTypeExternInject) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>();
	container.add<TestExternInject, cpv::Injector<TestExternInject, seastar::shared_ptr<Base>>>();
	auto instance = container.get<TestExternInject>();
	ASSERT_EQ(instance.instance->getValue(), 1);
}

TEST(TestContainer, getSingleInstance) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	auto instanceA = container.get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA->getValue(), 1);
	auto instanceB = container.get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA.get(), instanceB.get());
}

TEST(TestContainer, getMultipleInstances) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	auto instances = container.get<std::vector<seastar::shared_ptr<Base>>>();
	ASSERT_EQ(instances.size(), 2);
	ASSERT_EQ(instances.at(0)->getValue(), 1);
	ASSERT_EQ(instances.at(1)->getValue(), 1);
	ASSERT_NE(instances.at(0).get(), instances.at(1).get());
}

TEST(TestContainer, getInstanceEmptyError) {
	cpv::Container container;
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<seastar::shared_ptr<Base>>(),
		"no service entry found");
}

TEST(TestContainer, getInstanceMultiError) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<seastar::shared_ptr<Base>>(),
		"more than 1 service entry found");
}

TEST(TestContainer, getInstanceInjectError) {
	cpv::Container container;
	container.add<TestSelfInject, TestSelfInject>();
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<TestSelfInject>(),
		"call factory error");
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<TestSelfInject>(),
		"no service entry found");
}

TEST(TestContainer, eachInstances) {
	cpv::Container container;
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	container.add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	std::size_t total = 0;
	container.each<seastar::shared_ptr<Base>>([&total] (const auto& instance) {
		total += instance->getValue();
	});
	ASSERT_EQ(total, 2);
}

