#include <CPVFramework/Container/Container.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	class TestService {
	public:
		virtual std::string name() const = 0;
	};

	class TestImplA : public TestService {
	public:
		std::string name() const override { return "ImplA"; }
	};
}

TEST(TestContainer, addTransientServiceWithImplType) {
	cpv::Container container;
	container.add<TestImplA, TestImplA>();
	auto instance = container.get<TestImplA>();
	ASSERT_EQ(instance.name(), "ImplA");
}

TEST(TestContainer, addPresistentServiceWithImplType) {
	// TODO
}

TEST(TestContainer, addStoragePresistentServiceWithImplType) {
	// TODO
}

TEST(TestContainer, addTransientUniquePtrServiceWithImplType) {
	// TODO
}

TEST(TestContainer, addPresistentSharedPtrServiceWithImplType) {
	// TODO
}

TEST(TestContainer, addTransientObjectServiceWithImplType) {
	// TODO
}

TEST(TestContainer, addPresistentServiceWithInstance) {
	// TODO
}

TEST(TestContainer, addTransientServiceWithFunc2Args) {
	// TODO
}

TEST(TestContainer, addPresistentServiceWithFunc1Args) {
	// TODO
}

TEST(TestContainer, addStoragePresistentServiceWithFunc0Args) {
	// TODO
}

TEST(TestContainer, getServiceInstanceWithInjectedDependencies) {
	// TODO
}

TEST(TestContainer, getManyServiceInstanceWithInjectedDependencies) {
	// TODO
}

TEST(TestContainer, errorWhenGetServiceNotRegistered) {
	// TODO
}

TEST(TestContainer, errorWhenGetServiceMultipleRegistered) {
	// TODO
}

TEST(TestContainer, errorWhenGetPresistentServiceNotCopyConstructible) {
	// TODO
}

TEST(TestContainer, errorWhenGetStoragePresistentServiceNotCopyConstructible) {
	// TODO
}

TEST(TestContainer, patchTransientService) {
	// TODO
}

TEST(TestContainer, patchPresistentService) {
	// TODO
}

TEST(TestContainer, patchStoragePresistentService) {
	// TODO
}
