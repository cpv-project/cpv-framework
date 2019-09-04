#include <atomic>
#include <memory>
#include <seastar/core/sleep.hh>
#include <seastar/core/reactor.hh>
#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	class TestHandleStatesModule : public cpv::ModuleBase {
	public:
		seastar::future<> handle(cpv::Container& container, cpv::ApplicationState state) override {
			handleImpl(container, state);
			return seastar::sleep(std::chrono::milliseconds(1));
		}

	private:
		void handleImpl(cpv::Container& container, cpv::ApplicationState state) {
			// ASSERT_* can't use in function that return type isn't void
			if (state == cpv::ApplicationState::StartInitialize) {
				ASSERT_EQ(record, "");
				record.append("StartInitialize\n");
			} else if (state == cpv::ApplicationState::RegisterBasicServices) {
				container.add<int>(123);
				record.append("RegisterBasicServices\n");
			} else if (state == cpv::ApplicationState::BeforeCallCustomInitializeFunctions) {
				ASSERT_EQ(container.get<int>(), 123);
				record.append("BeforeCallCustomInitializeFunctions\n");
			} else if (state == cpv::ApplicationState::CallingCustomIntializeFunctions) {
				throw cpv::LogicException(CPV_CODEINFO,
					"should not invoke handler with this state:", state);
			} else if (state == cpv::ApplicationState::AfterCustomInitializeFunctionsCalled) {
				ASSERT_TRUE(initializeCount);
				ASSERT_TRUE(startCount);
				ASSERT_TRUE(stopCount);
				record.append("AfterCustomInitializeFunctionsCalled\n");
			} else if (state == cpv::ApplicationState::RegisterHeadServices) {
				record.append("RegisterHeadServices\n");
			} else if (state == cpv::ApplicationState::RegisterServices) {
				record.append("RegisterServices\n");
			} else if (state == cpv::ApplicationState::RegisterTailServices) {
				record.append("RegisterTailServices\n");
			} else if (state == cpv::ApplicationState::PatchServices) {
				record.append("PatchServices\n");
			} else if (state == cpv::ApplicationState::AfterServicesRegistered) {
				record.append("AfterServicesRegistered\n");
			} else if (state == cpv::ApplicationState::AfterInitialized) {
				record.append("AfterInitialized\n");
				ASSERT_EQ(record,
					"StartInitialize\n"
					"RegisterBasicServices\n"
					"BeforeCallCustomInitializeFunctions\n"
					"AfterCustomInitializeFunctionsCalled\n"
					"RegisterHeadServices\n"
					"RegisterServices\n"
					"RegisterTailServices\n"
					"PatchServices\n"
					"AfterServicesRegistered\n"
					"AfterInitialized\n");
				record.clear();
				initializeCount->fetch_add(1);
			} else if (state == cpv::ApplicationState::BeforeStart) {
				ASSERT_EQ(record, "");
				record.append("BeforeStart\n");
			} else if (state == cpv::ApplicationState::Starting) {
				record.append("Starting\n");
			} else if (state == cpv::ApplicationState::AfterStarted) {
				record.append("AfterStarted\n");
				ASSERT_EQ(record,
					"BeforeStart\n"
					"Starting\n"
					"AfterStarted\n");
				startCount->fetch_add(1);
			} else if (state == cpv::ApplicationState::BeforeStop) {
				record.append("BeforeStop\n");
			} else if (state == cpv::ApplicationState::Stopping) {
				record.append("Stopping\n");
			} else if (state == cpv::ApplicationState::AfterStopped) {
				record.append("AfterStopped\n");
				ASSERT_EQ(record,
					"BeforeStart\n"
					"Starting\n"
					"AfterStarted\n"
					"BeforeStop\n"
					"Stopping\n"
					"AfterStopped\n");
				record.clear();
				stopCount->fetch_add(1);
			}
		}

	public:
		std::shared_ptr<std::atomic_uint> initializeCount;
		std::shared_ptr<std::atomic_uint> startCount;
		std::shared_ptr<std::atomic_uint> stopCount;
		std::string record;
	};

	template <std::size_t Index, std::size_t MaxIndex>
	class TestHandleOrderModule : public cpv::ModuleBase {
	public:
		seastar::future<> handle(cpv::Container& container, cpv::ApplicationState state) override {
			handleImpl(container, state);
			if (Index % 2 == 0) {
				return seastar::make_ready_future<>();
			}
			return seastar::sleep(std::chrono::milliseconds(1));
		}

	private:
		void handleImpl(cpv::Container& container, cpv::ApplicationState state) {
			// ASSERT_* can't use in function that return type isn't void
			if (state == cpv::ApplicationState::StartInitialize && Index == 0) {
				container.add<seastar::shared_ptr<std::size_t>>(
					seastar::make_shared<std::size_t>(0));
				return;
			}
			auto value = container.get<seastar::shared_ptr<std::size_t>>();
			ASSERT_EQ(*value, (Index == 0) ? MaxIndex : (Index - 1));
			*value = Index;
		}
	};
}

TEST_FUTURE(TestApplication, empty) {
	cpv::Application application;
	return application.start().then([application] () mutable {
		return application.stop();
	});
}

TEST_FUTURE(TestApplication, handleStates) {
	return seastar::do_with(
		cpv::Application(),
		std::make_shared<std::atomic_uint>(),
		std::make_shared<std::atomic_uint>(),
		std::make_shared<std::atomic_uint>(),
		[] (cpv::Application& application,
		auto& initializeCount, auto& startCount, auto& stopCount) {
		application.add<TestHandleStatesModule>(
			[&initializeCount, &startCount, &stopCount] (auto& module) {
			module.initializeCount = initializeCount;
			module.startCount = startCount;
			module.stopCount = stopCount;
		});
		return application.start().then([&application] {
			return application.stop();
		}).then([&application] {
			return application.start();
		}).then([&application] {
			return application.stop();
		}).then([&initializeCount, &startCount, &stopCount] {
			ASSERT_EQ(initializeCount->load(), seastar::smp::count);
			ASSERT_EQ(startCount->load(), seastar::smp::count * 2);
			ASSERT_EQ(stopCount->load(), seastar::smp::count * 2);
		});
	});
}

TEST_FUTURE(TestApplication, handleOrder) {
	cpv::Application application;
	application.add<TestHandleOrderModule<0, 5>>();
	application.add<TestHandleOrderModule<1, 5>>();
	application.add<TestHandleOrderModule<2, 5>>();
	application.add<TestHandleOrderModule<3, 5>>();
	application.add<TestHandleOrderModule<4, 5>>();
	application.add<TestHandleOrderModule<5, 5>>();
	return application.start().then([application] () mutable {
		return application.stop();
	});
}

