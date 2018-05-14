#include <CPVFramework/Container/Aop.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	struct Base {
		virtual std::size_t getValue() = 0;
	};

	struct Derived : public Base {
		std::size_t getValue() override { return 1; }
	};

	struct Patcher : public Base {
		seastar::shared_ptr<Base> patchee;
		std::size_t getValue() override { return patchee->getValue() + 1; }
		Patcher(seastar::shared_ptr<Base>&& instance) : patchee(std::move(instance)) { }
	};

	struct TestObjectBase {
		virtual std::size_t getValue() = 0;
	};

	struct TestObjectDerived : public TestObjectBase {
		static void reset() { }
		static void freeResources() { }
		std::size_t getValue() override { return 1; }
	};

	struct TestObjectPatcher : public TestObjectBase {
		cpv::Object<TestObjectBase> patchee;
		void reset(cpv::Object<TestObjectBase>&& instance) {
			patchee = std::move(instance);
		}
		static void freeResources() { }
		std::size_t getValue() override { return patchee->getValue() + 1; }
	};
}

TEST(TestAop, patchInstance) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<Base>>(seastar::make_shared<Derived>());
	cpv::Aop::patch<seastar::shared_ptr<Base>>(*container,
		[] (const cpv::Container&, seastar::shared_ptr<Base>&& instance) {
			return seastar::make_shared<Patcher>(std::move(instance));
		});
	auto instanceA = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA->getValue(), 2);
	auto instanceB = container->get<seastar::shared_ptr<Base>>();
	ASSERT_EQ(instanceA.get(), instanceB.get());
}

TEST(TestAop, patchSingletonError) {
	auto container = cpv::Container::create();
	container->add<std::unique_ptr<Derived>>(std::make_unique<Derived>());
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		cpv::Aop::patch<std::unique_ptr<Derived>>(*container,
			[] (const cpv::Container&, std::unique_ptr<Derived>&& instance) {
				return std::move(instance);
			}),
		"can't patch singleton service that's not copy constructible");
}

TEST(TestAop, patchFactory) {
	auto container = cpv::Container::create();
	container->add<cpv::Object<TestObjectBase>, cpv::Object<TestObjectDerived>>();
	cpv::Aop::patch<cpv::Object<TestObjectBase>>(*container,
		[] (const cpv::Container&, cpv::Object<TestObjectBase>&& instance) {
			return cpv::makeObject<TestObjectPatcher>(std::move(instance)).cast<TestObjectBase>();
		});
	auto instanceA = container->get<cpv::Object<TestObjectBase>>();
	ASSERT_EQ(instanceA->getValue(), 2);
	auto instanceB = container->get<cpv::Object<TestObjectBase>>();
	ASSERT_NE(instanceA.get(), instanceB.get());
}

