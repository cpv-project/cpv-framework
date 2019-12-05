#include <CPVFramework/Logging/Logger.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(Logger, logLevel) {
	auto logger = cpv::Logger::createConsole(cpv::LogLevel::Warning);
	ASSERT_EQ(logger->getLogLevel(), cpv::LogLevel::Warning);
	ASSERT_TRUE(logger->isEnabled(cpv::LogLevel::Error));
	ASSERT_TRUE(logger->isEnabled(cpv::LogLevel::Warning));
	ASSERT_FALSE(logger->isEnabled(cpv::LogLevel::Notice));

	logger->setLogLevel(cpv::LogLevel::Info);
	ASSERT_EQ(logger->getLogLevel(), cpv::LogLevel::Info);
	ASSERT_TRUE(logger->isEnabled(cpv::LogLevel::Notice));
	ASSERT_TRUE(logger->isEnabled(cpv::LogLevel::Info));
	ASSERT_FALSE(logger->isEnabled(cpv::LogLevel::Debug));
}

TEST(Logger, create) {
	{
		auto logger = cpv::Logger::createConsole(cpv::LogLevel::Warning);
		ASSERT_TRUE(logger != nullptr);
	}
	{
		auto logger = cpv::Logger::createNoop();
		ASSERT_TRUE(logger != nullptr);
	}
}

