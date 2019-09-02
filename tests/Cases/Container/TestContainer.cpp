#include <sstream>
#include <CPVFramework/Container/Container.hpp>
#include <CPVFramework/Container/ServicePatcher.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	class TestService {
	public:
		virtual std::string name() const = 0;
		virtual ~TestService() = default;
	};

	class TestImplSimple : public TestService {
	public:
		std::string name() const override { return "ImplSimple"; }
	};

	class TestImplReusable : public TestService {
	public:
		std::string name() const override { return "ImplReusable"; }
		static void reset() { }
		static void freeResources() { }
	};

	class TestImplCustomName : public TestService {
	public:
		std::string name() const override { return name_; }
		explicit TestImplCustomName(std::string name) : name_(std::move(name)) { }
	private:
		std::string name_;
	};

	class TestImplInject : public TestService {
	public:
		using DependencyTypes = std::tuple<int, std::string, std::vector<std::unique_ptr<int>>>;
		std::string name() const override {
			std::ostringstream s;
			s << a_ << " " << b_ << " ";
			for (auto& ptr : c_) {
				(ptr == nullptr ? (s << "nullptr") : (s << *ptr)) << " ";
			}
			return s.str();
		}
		explicit TestImplInject(int a, std::string b, std::vector<std::unique_ptr<int>> c) :
			a_(a), b_(std::move(b)), c_(std::move(c)) { }
	private:
		int a_;
		std::string b_;
		std::vector<std::unique_ptr<int>> c_;
	};
}

template <>
thread_local cpv::ReusableStorageType<TestImplReusable>
	cpv::ReusableStorageInstance<TestImplReusable>;

TEST(TestContainer, addTransientServiceWithImplType) {
	cpv::Container container;
	container.add<TestImplSimple, TestImplSimple>();
	auto instance = container.get<TestImplSimple>();
	ASSERT_EQ(instance.name(), "ImplSimple");
}

TEST(TestContainer, addPersistentServiceWithImplType) {
	cpv::Container container;
	cpv::ServiceStorage storageP;
	container.add<
		seastar::shared_ptr<TestService>,
		seastar::shared_ptr<TestImplSimple>>(
		cpv::ServiceLifetime::Persistent);
	auto instanceFirst = container.get<seastar::shared_ptr<TestService>>();
	auto instanceSecond = container.get<seastar::shared_ptr<TestService>>();
	auto instanceThird = container.get<seastar::shared_ptr<TestService>>(storageP);
	ASSERT_TRUE(instanceFirst.get() != nullptr);
	ASSERT_EQ(instanceFirst.get(), instanceSecond.get());
	ASSERT_EQ(instanceFirst.get(), instanceThird.get());
	ASSERT_EQ(instanceFirst->name(), "ImplSimple");
}

TEST(TestContainer, addStoragePersistentServiceWithImplType) {
	cpv::Container container;
	cpv::ServiceStorage storageP;
	cpv::ServiceStorage storageQ;
	container.add<
		seastar::shared_ptr<TestService>,
		seastar::shared_ptr<TestImplSimple>>(
		cpv::ServiceLifetime::StoragePersistent);
	auto instanceBuiltinFirst = container.get<seastar::shared_ptr<TestService>>();
	auto instanceBuiltinSecond = container.get<seastar::shared_ptr<TestService>>();
	auto instancePFirst = container.get<seastar::shared_ptr<TestService>>(storageP);
	auto instancePSecond = container.get<seastar::shared_ptr<TestService>>(storageP);
	auto instanceQFirst = container.get<seastar::shared_ptr<TestService>>(storageQ);
	auto instanceQSecond = container.get<seastar::shared_ptr<TestService>>(storageQ);
	ASSERT_TRUE(instanceBuiltinFirst.get() != nullptr);
	ASSERT_EQ(instanceBuiltinFirst.get(), instanceBuiltinSecond.get());
	ASSERT_TRUE(instancePFirst.get() != nullptr);
	ASSERT_EQ(instancePFirst.get(), instancePSecond.get());
	ASSERT_TRUE(instanceQFirst.get() != nullptr);
	ASSERT_EQ(instanceQFirst.get(), instanceQSecond.get());
	ASSERT_NE(instanceBuiltinFirst.get(), instancePFirst.get());
	ASSERT_NE(instanceBuiltinFirst.get(), instanceQFirst.get());
	ASSERT_NE(instancePFirst.get(), instanceQFirst.get());
	ASSERT_EQ(instanceBuiltinFirst->name(), "ImplSimple");
	ASSERT_EQ(instancePFirst->name(), "ImplSimple");
	ASSERT_EQ(instanceQFirst->name(), "ImplSimple");
}

TEST(TestContainer, addTransientUniquePtrServiceWithImplType) {
	cpv::Container container;
	container.add<std::unique_ptr<TestService>, std::unique_ptr<TestImplSimple>>();
	auto instanceFirst = container.get<std::unique_ptr<TestService>>();
	auto instanceSecond = container.get<std::unique_ptr<TestService>>();
	ASSERT_NE(instanceFirst.get(), instanceSecond.get());
	ASSERT_EQ(instanceFirst->name(), "ImplSimple");
	ASSERT_EQ(instanceSecond->name(), "ImplSimple");
}

TEST(TestContainer, addPersistentSharedPtrServiceWithImplType) {
	cpv::Container container;
	container.add<
		seastar::shared_ptr<TestService>,
		seastar::shared_ptr<TestImplSimple>>(
		cpv::ServiceLifetime::Persistent);
	auto instanceFirst = container.get<seastar::shared_ptr<TestService>>();
	auto instanceSecond = container.get<seastar::shared_ptr<TestService>>();
	ASSERT_TRUE(instanceFirst.get() != nullptr);
	ASSERT_EQ(instanceFirst.get(), instanceSecond.get());
	ASSERT_EQ(instanceFirst->name(), "ImplSimple");
}

TEST(TestContainer, addTransientReusableServiceWithImplType) {
	cpv::Container container;
	container.add<
		cpv::Reusable<TestService>,
		cpv::Reusable<TestImplReusable>>();
	auto instanceFirst = container.get<cpv::Reusable<TestService>>();
	auto instanceSecond = container.get<cpv::Reusable<TestService>>();
	ASSERT_NE(instanceFirst.get(), instanceSecond.get());
	ASSERT_EQ(instanceFirst->name(), "ImplReusable");
	ASSERT_EQ(instanceSecond->name(), "ImplReusable");
}

TEST(TestContainer, addPersistentServiceWithInstance) {
	cpv::Container container;
	container.add<seastar::shared_ptr<TestService>>(
		seastar::make_shared<TestImplCustomName>(
			"TestAddPersistentServiceWithInstance"));
	auto instanceFirst = container.get<seastar::shared_ptr<TestService>>();
	auto instanceSecond = container.get<seastar::shared_ptr<TestService>>();
	ASSERT_TRUE(instanceFirst.get() != nullptr);
	ASSERT_EQ(instanceFirst.get(), instanceSecond.get());
	ASSERT_EQ(instanceFirst->name(), "TestAddPersistentServiceWithInstance");
}

TEST(TestContainer, addTransientServiceWithFunc2Args) {
	cpv::Container container;
	cpv::ServiceStorage storageP;
	container.add<
		seastar::shared_ptr<TestImplSimple>,
		seastar::shared_ptr<TestImplSimple>>(
		cpv::ServiceLifetime::StoragePersistent);
	container.add<seastar::shared_ptr<TestService>>(
		[] (const cpv::Container& container, cpv::ServiceStorage& storage) {
			return container.get<seastar::shared_ptr<TestImplSimple>>(storage);
		});
	auto instanceBuiltinFirst = container.get<seastar::shared_ptr<TestService>>();
	auto instanceBuiltinSecond = container.get<seastar::shared_ptr<TestImplSimple>>();
	auto instancePFirst = container.get<seastar::shared_ptr<TestService>>(storageP);
	auto instancePSecond = container.get<seastar::shared_ptr<TestImplSimple>>(storageP);
	ASSERT_TRUE(instanceBuiltinFirst.get() != nullptr);
	ASSERT_EQ(instanceBuiltinFirst.get(), instanceBuiltinSecond.get());
	ASSERT_TRUE(instancePFirst.get() != nullptr);
	ASSERT_EQ(instancePFirst.get(), instancePSecond.get());
	ASSERT_NE(instanceBuiltinFirst.get(), instancePFirst.get());
	ASSERT_EQ(instanceBuiltinFirst->name(), "ImplSimple");
	ASSERT_EQ(instancePFirst->name(), "ImplSimple");
}

TEST(TestContainer, addPersistentServiceWithFunc1Args) {
	cpv::Container container;
	cpv::ServiceStorage storageP;
	container.add<
		seastar::shared_ptr<TestImplSimple>,
		seastar::shared_ptr<TestImplSimple>>();
	container.add<seastar::shared_ptr<TestService>>(
		[] (const cpv::Container& container) {
			return container.get<seastar::shared_ptr<TestImplSimple>>();
		}, cpv::ServiceLifetime::Persistent);
	auto instanceFirst = container.get<seastar::shared_ptr<TestService>>();
	auto instanceSecond = container.get<seastar::shared_ptr<TestService>>();
	auto instanceThird = container.get<seastar::shared_ptr<TestImplSimple>>();
	ASSERT_TRUE(instanceFirst.get() != nullptr);
	ASSERT_EQ(instanceFirst.get(), instanceSecond.get());
	ASSERT_NE(instanceFirst.get(), instanceThird.get());
	ASSERT_EQ(instanceFirst->name(), "ImplSimple");
	ASSERT_EQ(instanceThird->name(), "ImplSimple");
}

TEST(TestContainer, addStoragePersistentServiceWithFunc0Args) {
	cpv::Container container;
	cpv::ServiceStorage storageP;
	cpv::ServiceStorage storageQ;
	container.add<seastar::shared_ptr<TestService>>(
		[] {
			return seastar::make_shared<TestImplSimple>();
		}, cpv::ServiceLifetime::StoragePersistent);
	auto instanceBuiltinFirst = container.get<seastar::shared_ptr<TestService>>();
	auto instanceBuiltinSecond = container.get<seastar::shared_ptr<TestService>>();
	auto instancePFirst = container.get<seastar::shared_ptr<TestService>>(storageP);
	auto instancePSecond = container.get<seastar::shared_ptr<TestService>>(storageP);
	auto instanceQFirst = container.get<seastar::shared_ptr<TestService>>(storageQ);
	auto instanceQSecond = container.get<seastar::shared_ptr<TestService>>(storageQ);
	ASSERT_TRUE(instanceBuiltinFirst.get() != nullptr);
	ASSERT_EQ(instanceBuiltinFirst.get(), instanceBuiltinSecond.get());
	ASSERT_TRUE(instancePFirst.get() != nullptr);
	ASSERT_EQ(instancePFirst.get(), instancePSecond.get());
	ASSERT_TRUE(instanceQFirst.get() != nullptr);
	ASSERT_EQ(instanceQFirst.get(), instanceQSecond.get());
	ASSERT_NE(instanceBuiltinFirst.get(), instancePFirst.get());
	ASSERT_NE(instanceBuiltinFirst.get(), instanceQFirst.get());
	ASSERT_NE(instancePFirst.get(), instanceQFirst.get());
	ASSERT_EQ(instanceBuiltinFirst->name(), "ImplSimple");
	ASSERT_EQ(instancePFirst->name(), "ImplSimple");
	ASSERT_EQ(instanceQFirst->name(), "ImplSimple");
}

TEST(TestContainer, getServiceInstanceWithInjectedDependencies) {
	cpv::Container container;
	// add service first to ensure descriptors are updated to date
	container.add<std::unique_ptr<TestService>, std::unique_ptr<TestImplInject>>();
	container.add<int>(123);
	container.add<std::string>("abc");
	container.add<std::unique_ptr<int>>([] { return std::make_unique<int>(100); });
	container.add<std::unique_ptr<int>>([] { return nullptr; });
	container.add<std::unique_ptr<int>>([] { return std::make_unique<int>(101); });
	auto instanceFirst = container.get<std::unique_ptr<TestService>>();
	auto instanceSecond = container.get<std::unique_ptr<TestService>>();
	ASSERT_TRUE(instanceFirst.get() != nullptr);
	ASSERT_TRUE(instanceSecond.get() != nullptr);
	ASSERT_NE(instanceFirst.get(), instanceSecond.get());
	ASSERT_EQ(instanceFirst->name(), "123 abc 100 nullptr 101 ");
	ASSERT_EQ(instanceSecond->name(), "123 abc 100 nullptr 101 ");
}

TEST(TestContainer, getManyServiceIntoVector) {
	cpv::Container container;
	container.add<
		seastar::shared_ptr<TestService>,
		seastar::shared_ptr<TestImplSimple>>();
	container.add<seastar::shared_ptr<TestService>>(
		[] {
			return seastar::make_shared<TestImplCustomName>(
				"TestGetManyServiceIntoVector");
		}, cpv::ServiceLifetime::Persistent);
	std::vector<seastar::shared_ptr<TestService>> instancesFirst;
	std::vector<seastar::shared_ptr<TestService>> instancesSecond;
	container.getMany(instancesFirst);
	container.getMany(instancesSecond);
	ASSERT_EQ(instancesFirst.size(), 2U);
	ASSERT_EQ(instancesSecond.size(), 2U);
	ASSERT_TRUE(instancesFirst.at(0).get() != nullptr);
	ASSERT_TRUE(instancesFirst.at(1).get() != nullptr);
	ASSERT_TRUE(instancesSecond.at(0).get() != nullptr);
	ASSERT_NE(instancesFirst.at(0).get(), instancesSecond.at(0).get());
	ASSERT_EQ(instancesFirst.at(1).get(), instancesSecond.at(1).get());
	ASSERT_EQ(instancesFirst.at(0)->name(), "ImplSimple");
	ASSERT_EQ(instancesFirst.at(1)->name(), "TestGetManyServiceIntoVector");
	ASSERT_EQ(instancesSecond.at(0)->name(), "ImplSimple");
}

TEST(TestContainer, getManyServiceIntoStackAllocatedVectorWithStorage) {
	using VectorType = cpv::StackAllocatedVector<seastar::shared_ptr<TestService>, 16>;
	cpv::Container container;
	cpv::ServiceStorage storageP;
	cpv::ServiceStorage storageQ;
	container.add<
		seastar::shared_ptr<TestService>,
		seastar::shared_ptr<TestImplSimple>>(
		cpv::ServiceLifetime::Persistent);
	container.add<seastar::shared_ptr<TestService>>(
		[] {
			return seastar::make_shared<TestImplCustomName>(
				"TestGetManyServiceIntoVector");
		}, cpv::ServiceLifetime::StoragePersistent);
	VectorType instancesBuiltinFirst;
	VectorType instancesBuiltinSecond;
	VectorType instancesPFirst;
	VectorType instancesPSecond;
	VectorType instancesQFirst;
	VectorType instancesQSecond;
	container.getMany(instancesBuiltinFirst);
	container.getMany(instancesBuiltinSecond);
	container.getMany(instancesPFirst, storageP);
	container.getMany(instancesPSecond, storageP);
	container.getMany(instancesQFirst, storageQ);
	container.getMany(instancesQSecond, storageQ);
	ASSERT_EQ(instancesBuiltinFirst.size(), 2U);
	ASSERT_EQ(instancesBuiltinSecond.size(), 2U);
	ASSERT_EQ(instancesPFirst.size(), 2U);
	ASSERT_EQ(instancesPSecond.size(), 2U);
	ASSERT_EQ(instancesQFirst.size(), 2U);
	ASSERT_EQ(instancesQSecond.size(), 2U);
	ASSERT_EQ(instancesBuiltinFirst.at(0).get(), instancesBuiltinSecond.at(0).get());
	ASSERT_EQ(instancesBuiltinFirst.at(0).get(), instancesPFirst.at(0).get());
	ASSERT_EQ(instancesBuiltinFirst.at(0).get(), instancesPSecond.at(0).get());
	ASSERT_EQ(instancesBuiltinFirst.at(0).get(), instancesQFirst.at(0).get());
	ASSERT_EQ(instancesBuiltinFirst.at(0).get(), instancesQSecond.at(0).get());
	ASSERT_EQ(instancesBuiltinFirst.at(1).get(), instancesBuiltinSecond.at(1).get());
	ASSERT_EQ(instancesPFirst.at(1).get(), instancesPSecond.at(1).get());
	ASSERT_EQ(instancesQFirst.at(1).get(), instancesQSecond.at(1).get());
	ASSERT_NE(instancesBuiltinFirst.at(1).get(), instancesPFirst.at(1).get());
	ASSERT_NE(instancesBuiltinFirst.at(1).get(), instancesQFirst.at(1).get());
	ASSERT_NE(instancesPFirst.at(1).get(), instancesQFirst.at(1).get());
	ASSERT_EQ(instancesBuiltinFirst.at(0)->name(), "ImplSimple");
	ASSERT_EQ(instancesBuiltinFirst.at(1)->name(), "TestGetManyServiceIntoVector");
	ASSERT_EQ(instancesPFirst.at(1)->name(), "TestGetManyServiceIntoVector");
	ASSERT_EQ(instancesQFirst.at(1)->name(), "TestGetManyServiceIntoVector");
}

TEST(TestContainer, getManyServiceIntoVectorMultipleTimes) {
	cpv::Container container;
	container.add<int>(1);
	container.add<int>(2);
	container.add<int>(3);
	std::vector<int> instances;
	container.getMany(instances);
	container.getMany(instances);
	ASSERT_EQ(instances.size(), 6U);
	ASSERT_EQ(instances.at(0), 1);
	ASSERT_EQ(instances.at(1), 2);
	ASSERT_EQ(instances.at(2), 3);
	ASSERT_EQ(instances.at(3), 1);
	ASSERT_EQ(instances.at(4), 2);
	ASSERT_EQ(instances.at(5), 3);
}

TEST(TestContainer, getVectorAsSingleService) {
	cpv::Container container;
	container.add(std::vector<int>({ 1, 2, 3}));
	auto instance = container.get<std::vector<int>>();
	ASSERT_EQ(instance.size(), 3U);
	ASSERT_EQ(instance.at(0), 1);
	ASSERT_EQ(instance.at(1), 2);
	ASSERT_EQ(instance.at(2), 3);
}

TEST(TestContainer, errorWhenGetServiceNotRegistered) {
	cpv::Container container;
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<int>(),
		"failed: not registered");
}

TEST(TestContainer, errorWhenGetServiceMultipleRegistered) {
	cpv::Container container;
	container.add<int>(1);
	container.add<int>(2);
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<int>(),
		"failed: registered multiple times");
}

TEST(TestContainer, errorWhenGetPersistentServiceNotCopyConstructible) {
	cpv::Container container;
	container.add<
		std::unique_ptr<TestService>,
		std::unique_ptr<TestImplSimple>>(
		cpv::ServiceLifetime::Persistent);
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<std::unique_ptr<TestService>>(),
		"error: lifetime is Persistent but not copy constructible");
}

TEST(TestContainer, errorWhenGetStoragePersistentServiceNotCopyConstructible) {
	cpv::Container container;
	container.add<
		std::unique_ptr<TestService>,
		std::unique_ptr<TestImplSimple>>(
		cpv::ServiceLifetime::StoragePersistent);
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<std::unique_ptr<TestService>>(),
		"error: lifetime is storage persistent but not copy constructible");
}

TEST(TestContainer, patchTransientServiceWithFunc0Args) {
	cpv::Container container;
	container.add<std::unique_ptr<int>>([] {
		return std::make_unique<int>(1);
	});
	auto count = seastar::make_shared<std::size_t>(0);
	cpv::ServicePatcher<std::unique_ptr<int>>::patch(
		container, [count] (std::unique_ptr<int> v) {
			++(*count);
			*v = -*v;
			return v;
		});
	ASSERT_EQ(*container.get<std::unique_ptr<int>>(), -1);
	ASSERT_EQ(*container.get<std::unique_ptr<int>>(), -1);
	ASSERT_EQ(*count, 2U);
}

TEST(TestContainer, patchPersistentServiceWithFunc1Args) {
	cpv::Container container;
	container.add<int>(100);
	container.add<seastar::shared_ptr<int>>([] {
		return seastar::make_shared<int>(1);
	}, cpv::ServiceLifetime::Persistent);
	auto count = seastar::make_shared<std::size_t>(0);
	cpv::ServicePatcher<seastar::shared_ptr<int>>::patch(
		container, [count] (const cpv::Container& c, seastar::shared_ptr<int> v) {
			++(*count);
			*v += c.get<int>();
			return v;
		});
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(), 101);
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(), 101);
	ASSERT_EQ(*count, 1U);
}

TEST(TestContainer, patchStoragePersistentServiceWithFunc2Args) {
	cpv::Container container;
	cpv::ServiceStorage storageP;
	cpv::ServiceStorage storageQ;
	container.add<int>([v=seastar::make_shared<int>(0)] {
		return ++*v;
	}, cpv::ServiceLifetime::StoragePersistent);
	container.add<seastar::shared_ptr<int>>([] {
		return seastar::make_shared<int>(100);
	}, cpv::ServiceLifetime::StoragePersistent);
	auto count = seastar::make_shared<std::size_t>(0);
	cpv::ServicePatcher<seastar::shared_ptr<int>>::patch(
		container, [count] (
			const cpv::Container& c, cpv::ServiceStorage& s, seastar::shared_ptr<int> v) {
			++(*count);
			*v += c.get<int>(s);
			return v;
		});
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(), 101);
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(), 101);
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(storageP), 102);
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(storageP), 102);
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(storageQ), 103);
	ASSERT_EQ(*container.get<seastar::shared_ptr<int>>(storageQ), 103);
	ASSERT_EQ(*count, 3U);
}

TEST(TestContainer, patchNotRegisteredService) {
	cpv::Container container;
	cpv::ServicePatcher<int>::patch(container, [] (int) { return 0; });
	ASSERT_THROWS_CONTAINS(
		cpv::ContainerException,
		container.get<int>(),
		"failed: not registered");
}

TEST(TestContainer, patchDoesNotBreakDIFactory) {
	cpv::Container container;
	container.add<std::unique_ptr<TestService>, std::unique_ptr<TestImplInject>>();
	container.add<int>(123);
	container.add<std::string>("abc");
	container.add<std::unique_ptr<int>>([] { return std::make_unique<int>(100); });
	container.add<std::unique_ptr<int>>([] { return nullptr; });
	container.add<std::unique_ptr<int>>([] { return std::make_unique<int>(101); });
	cpv::ServicePatcher<std::string>::patch(
		container, [] (std::string v) { return v + ".patched"; });
	cpv::ServicePatcher<std::unique_ptr<int>>::patch(
		container, [] (std::unique_ptr<int> v) {
			if (v != nullptr) {
				*v = -*v;
			}
			return v;
		});
	auto instance = container.get<std::unique_ptr<TestService>>();
	ASSERT_TRUE(instance.get() != nullptr);
	ASSERT_EQ(instance->name(), "123 abc.patched -100 nullptr -101 ");
}

