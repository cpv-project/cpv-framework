#pragma once
#include <gtest/gtest.h>

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

namespace cpv {
	namespace Internal_Gtest {
		int runAllTests(int argc, char** argv);
	}
}

namespace {
	/** Construct string with fixed size, string may contains \x00 */
	template <std::size_t Size>
	std::string makeTestString(const char(&str)[Size]) {
		static_assert(Size > 0, "string must be null terminated");
		return std::string(str, Size - 1);
	}
}

