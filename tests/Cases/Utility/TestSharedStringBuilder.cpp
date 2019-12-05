#include <CPVFramework/Utility/SharedStringBuilder.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(SharedStringBuilder, grow) {
	cpv::SharedStringBuilder builder;
	ASSERT_TRUE(builder.grow(12) == builder.data());
	ASSERT_EQ(builder.capacity(), 12U);
	ASSERT_EQ(builder.size(), 12U);
	ASSERT_TRUE(builder.grow(13) == builder.data() + 12);
	ASSERT_EQ(builder.capacity(), cpv::SharedStringBuilder::MinCapacityForGrow);
	ASSERT_EQ(builder.size(), 25U);
	ASSERT_TRUE(builder.grow(1) == builder.data() + 25);
	ASSERT_EQ(builder.capacity(), cpv::SharedStringBuilder::MinCapacityForGrow);
	ASSERT_EQ(builder.size(), 26U);
}

TEST(SharedStringBuilder, growOverflow) {
	cpv::SharedStringBuilder builder("ab");
	ASSERT_THROWS_CONTAINS(
		cpv::OverflowException,
		builder.grow(std::numeric_limits<std::size_t>::max() - 1),
		"size overflowed");
}

TEST(SharedStringBuilder, append) {
	cpv::SharedStringBuilder builder;
	builder.append("test|")
		.append(3, 'a') .append(1, '|')
		.append("abc|")
		.append(123).append("|")
		.append(12345678).append("|")
		.append(-123).append("|")
		.append(1.0).append("|")
		.append(static_cast<long double>(10.2)).append("|")
		.append(100.0);
	cpv::SharedString str = builder.build();
	ASSERT_EQ(builder.capacity(), 0U);
	ASSERT_EQ(builder.size(), 0U);
	ASSERT_EQ(str, "test|aaa|abc|123|12345678|-123|1|10.2|100");
}

TEST(SharedStringBuilder, resize) {
	cpv::SharedStringBuilder builder;
	builder.append("test|").append("abc|").append("def|");
	builder.resize(11);
	cpv::SharedString str = builder.build();
	ASSERT_EQ(str, "test|abc|de");
}

TEST(SharedStringBuilder, misc) {
	cpv::SharedStringBuilder builder;
	builder.append("test|").append("abc");
	ASSERT_EQ(std::string_view(builder.data(), builder.size()), "test|abc");
	ASSERT_EQ(std::string(builder.begin(), builder.end()), "test|abc");
	ASSERT_EQ(builder.view(), "test|abc");
	builder.clear();
	ASSERT_EQ(builder.view(), "");
}

