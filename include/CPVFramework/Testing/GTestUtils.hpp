#pragma once
#include <string>
#include <gtest/gtest.h>
#include <seastar/core/app-template.hh>
#include <seastar/core/future.hh>
#include <seastar/core/future-util.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/thread.hh>
#include "../Utility/Packet.hpp"

#define TEST_FUTURE(caseName, testName) \
	static seastar::future<> caseName##_##testName##_FutureTestBody(); \
	TEST(caseName, testName) { \
		seastar::future<> f(caseName##_##testName##_FutureTestBody());\
		f.get(); \
	} \
	static seastar::future<> caseName##_##testName##_FutureTestBody()

#define ASSERT_THROWS_CONTAINS(exception, expression, contains) \
	do { \
		try { expression; } \
		catch (const exception& ex) { \
			std::string message(ex.what()); \
			if (message.find(contains) == std::string::npos) { \
				FAIL() << "exception message didn't contains expected  words: " << message; \
			} \
			break; \
		} \
		catch (...) { throw; } \
		FAIL() << "No exception throws"; \
	} while (0)

#define ASSERT_CONTAINS(str, pattern) \
	do { \
		if ((str).find(pattern) == std::string::npos) { \
			FAIL() << "string didn't contains expected words: " << str; \
		} \
	} while (0)

#define ASSERT_THROWS(exception, expression) ASSERT_THROWS_CONTAINS(exception, expression, "")

namespace cpv::gtest {
	/** the main function of test executable */
	static inline int runAllTests(int argc, char** argv) {
		::testing::InitGoogleTest(&argc, argv);
		seastar::app_template app;
		int returnValue(0);
		app.run(argc, argv, [&returnValue] {
			return seastar::async([] {
				return RUN_ALL_TESTS();
			}).then([&returnValue] (int result) {
				returnValue = result;
				// wait for internal cleanup to make leak sanitizer happy
				return seastar::sleep(std::chrono::seconds(1));
			});
		});
		return returnValue;
	}
	
	/** create tcp connection, send request then return received response as string */
	seastar::future<std::string> tcpSendRequest(
		const std::string& ip, std::size_t port, Packet&& p);
	
	/** create tcp connection, send request partially then return received response as string */
	seastar::future<std::string> tcpSendPartialRequest(
		const std::string& ip,
		std::size_t port,
		const std::vector<std::string_view>& parts,
		std::chrono::milliseconds interval);
}

