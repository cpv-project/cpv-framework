#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Application/Modules/LoggingModule.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	template <cpv::LogLevel Level>
	class TestLoggerModule : public cpv::ModuleBase {
	public:
		seastar::future<> handle(cpv::Container& container, cpv::ApplicationState state) override {
			if (state == cpv::ApplicationState::AfterServicesRegistered) {
				[&container] {
					auto logger = container.get<seastar::shared_ptr<cpv::Logger>>();
					ASSERT_TRUE(logger);
					ASSERT_EQ(logger->getLogLevel(), Level);
				}();
			}
			return seastar::make_ready_future<>();
		}
	};
}

TEST_FUTURE(LoggingModule, getDefaultLogger) {
	cpv::Application application;
	application.add<cpv::LoggingModule>();
	application.add<TestLoggerModule<cpv::LogLevel::Notice>>();
	return application.start().then([application] () mutable {
		return application.stop();
	});
}

TEST_FUTURE(LoggingModule, setCustomLogger) {
	cpv::Application application;
	application.add<cpv::LoggingModule>([] (auto& module) {
		module.setLogger(cpv::Logger::createConsole(cpv::LogLevel::Info));
	});
	application.add<TestLoggerModule<cpv::LogLevel::Info>>();
	return application.start().then([application] () mutable {
		return application.stop();
	});
}

