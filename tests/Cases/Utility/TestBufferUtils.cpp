#include <limits>
#include <CPVFramework/Exceptions/OverflowException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestBufferUtils, mergeContent) {
	seastar::temporary_buffer<char> buf;
	std::string_view exists("000");
	// initial merge
	cpv::mergeContent(buf, exists, "11111");
	ASSERT_EQ(exists, "00011111");
	ASSERT_EQ(exists.data(), buf.get());
	ASSERT_LE(exists.size(), buf.size());
	// capacity enough
	cpv::mergeContent(buf, exists, "0000");
	ASSERT_EQ(exists, "000111110000");
	ASSERT_EQ(exists.data(), buf.get());
	ASSERT_LE(exists.size(), buf.size());
	// capacity not enough
	std::string longContent(1024, 'a');
	cpv::mergeContent(buf, exists, longContent);
	ASSERT_EQ(exists, "000111110000" + longContent);
	ASSERT_EQ(exists.data(), buf.get());
	ASSERT_LE(exists.size(), buf.size());
}

TEST(TestBufferUtils, mergeContentError) {
	seastar::temporary_buffer<char> buf;
	std::string_view exists("000");
	ASSERT_THROWS_CONTAINS(
		cpv::OverflowException,
		cpv::mergeContent(buf, exists, std::string_view("", std::numeric_limits<std::size_t>::max() - 1)),
		"size of existsContent + newContent overflowed");
}

