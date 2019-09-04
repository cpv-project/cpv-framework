#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace cpv {
	enum class MyTestEnum {
		None = 0,
		A = 1,
		B = 2,
		C = 4
	};

	template <>
	struct EnumDescriptions<MyTestEnum> {
		static const std::vector<std::pair<MyTestEnum, const char*>>& get() {
			static std::vector<std::pair<MyTestEnum, const char*>> staticNames({
				{ MyTestEnum::None, "None" },
				{ MyTestEnum::A, "A" },
				{ MyTestEnum::B, "B" },
				{ MyTestEnum::C, "C" }
			});
			return staticNames;
		}
	};
}

TEST(TestEnumUtils, operators) {
	{
		auto value = cpv::MyTestEnum::A | cpv::MyTestEnum::B;
		ASSERT_EQ(static_cast<std::size_t>(value), 3U);
	}
	{
		auto value = cpv::MyTestEnum::A;
		value |= cpv::MyTestEnum::B;
		ASSERT_EQ(static_cast<std::size_t>(value), 3U);
	}
	{
		auto value = cpv::MyTestEnum::A & (cpv::MyTestEnum::A | cpv::MyTestEnum::B);
		ASSERT_EQ(static_cast<std::size_t>(value), 1U);
	}
	{
		auto value = cpv::MyTestEnum::A | cpv::MyTestEnum::B;
		value &= cpv::MyTestEnum::A;
		ASSERT_EQ(static_cast<std::size_t>(value), 1U);
	}
	{
		auto value = ~cpv::MyTestEnum::A & (cpv::MyTestEnum::A | cpv::MyTestEnum::B);
		ASSERT_EQ(static_cast<std::size_t>(value), 2U);
	}
	{
		auto value = cpv::MyTestEnum::A | cpv::MyTestEnum::B;
		value &= cpv::MyTestEnum::B;
		ASSERT_EQ(static_cast<std::size_t>(value), 2U);
	}
	{
		auto value = ~cpv::MyTestEnum::A;
		ASSERT_EQ(static_cast<std::size_t>(value), ~static_cast<std::size_t>(1));
	}
	{
		auto value = cpv::MyTestEnum::A | cpv::MyTestEnum::B;
		ASSERT_EQ(enumValue(value), 3);
	}
}

TEST(TestEnumUtils, enumTrue) {
	ASSERT_TRUE(enumTrue(cpv::MyTestEnum::A));
	ASSERT_FALSE(enumTrue(cpv::MyTestEnum::None));
}

TEST(TestEnumUtils, enumFalse) {
	ASSERT_TRUE(enumFalse(cpv::MyTestEnum::None));
	ASSERT_FALSE(enumFalse(cpv::MyTestEnum::A));
}

TEST(TestEnumUtils, enumToString) {
	ASSERT_EQ(cpv::joinString("", cpv::MyTestEnum::None), "None");
	ASSERT_EQ(cpv::joinString("", cpv::MyTestEnum::A | cpv::MyTestEnum::B), "A|B");
}

TEST(TestEnumUtils, enumFromString) {
	ASSERT_EQ(cpv::enumFromString<cpv::MyTestEnum>("A"), cpv::MyTestEnum::A);
	ASSERT_EQ(cpv::enumFromString<cpv::MyTestEnum>("B"), cpv::MyTestEnum::B);
	ASSERT_EQ(cpv::enumFromString<cpv::MyTestEnum>("C"), cpv::MyTestEnum::C);
	ASSERT_EQ(cpv::enumFromString<cpv::MyTestEnum>("D"), cpv::MyTestEnum::None);
	ASSERT_EQ(cpv::enumFromString<cpv::MyTestEnum>("D", cpv::MyTestEnum::A), cpv::MyTestEnum::A);
}

