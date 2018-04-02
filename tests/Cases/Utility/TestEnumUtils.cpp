#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

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
		ASSERT_EQ(static_cast<std::size_t>(value), 3);
	}
	{
		auto value = cpv::MyTestEnum::A;
		value |= cpv::MyTestEnum::B;
		ASSERT_EQ(static_cast<std::size_t>(value), 3);
	}
	{
		auto value = cpv::MyTestEnum::A & (cpv::MyTestEnum::A | cpv::MyTestEnum::B);
		ASSERT_EQ(static_cast<std::size_t>(value), 1);
	}
	{
		auto value = cpv::MyTestEnum::A | cpv::MyTestEnum::B;
		value &= cpv::MyTestEnum::A;
		ASSERT_EQ(static_cast<std::size_t>(value), 1);
	}
	{
		auto value = ~cpv::MyTestEnum::A & (cpv::MyTestEnum::A | cpv::MyTestEnum::B);
		ASSERT_EQ(static_cast<std::size_t>(value), 2);
	}
	{
		auto value = cpv::MyTestEnum::A | cpv::MyTestEnum::B;
		value &= cpv::MyTestEnum::B;
		ASSERT_EQ(static_cast<std::size_t>(value), 2);
	}
	{
		auto value = ~cpv::MyTestEnum::A;
		ASSERT_EQ(static_cast<std::size_t>(value), ~1);
	}
	{
		auto value = cpv::MyTestEnum::A;
		ASSERT_TRUE(enumTrue(value));
	}
	{
		auto value = cpv::MyTestEnum::None;
		ASSERT_TRUE(enumFalse(value));
	}
	{
		auto value = cpv::MyTestEnum::A | cpv::MyTestEnum::B;
		ASSERT_EQ(enumValue(value), 3);
	}
	{
		ASSERT_EQ(cpv::joinString("", cpv::MyTestEnum::None), "None");
		ASSERT_EQ(cpv::joinString("", cpv::MyTestEnum::A | cpv::MyTestEnum::B), "A|B");
	}
}

