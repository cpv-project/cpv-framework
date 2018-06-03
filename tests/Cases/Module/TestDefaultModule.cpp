#include <CPVFramework/Module/DefaultModule.hpp>
#include <CPVFramework/Utility/JsonUtils.hpp>
#include <CPVFramework/Logging/Logger.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestDefaultModule, logger) {
	{
		auto container = cpv::Container::create();
		cpv::Json configuration = cpv::Json::parse("{}");
		cpv::DefaultModule module(seastar::make_shared<cpv::Json>(configuration));
		module.registerServices(*container);
		auto logger = container->get<seastar::shared_ptr<cpv::Logger>>();
		ASSERT_EQ(logger->getLogLevel(), cpv::LogLevel::Info);
	}
	{
		auto container = cpv::Container::create();
		cpv::Json configuration;
		configuration["logging.log_level"] = "Error";
		cpv::DefaultModule module(seastar::make_shared<cpv::Json>(configuration));
		module.registerServices(*container);
		auto logger = container->get<seastar::shared_ptr<cpv::Logger>>();
		ASSERT_EQ(logger->getLogLevel(), cpv::LogLevel::Error);
	}
}

