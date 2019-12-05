#include <unistd.h>
#include <CPVFramework/Utility/FileUtils.hpp>
#include <CPVFramework/Exceptions/FileSystemException.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	static const std::string path("/tmp/cpv-framework-test-file-utils.txt");
}

TEST(FileUtils, readFile) {
	cpv::writeFile(path, "test contents");
	ASSERT_EQ(cpv::readFile(path), "test contents");
	::unlink(path.c_str());
}

TEST(FileUtils, readFileError) {
	::unlink(path.c_str());
	ASSERT_THROWS(
		cpv::FileSystemException,
		cpv::readFile(path));
}

TEST(FileUtils, writeFile) {
	cpv::writeFile(path, "test contents");
	ASSERT_EQ(cpv::readFile(path), "test contents");
	::unlink(path.c_str());
}

TEST(FileUtils, isSafePath) {
	ASSERT_TRUE(cpv::isSafePath("./dir/test.txt"));
	ASSERT_TRUE(cpv::isSafePath("./dir/.test.txt"));
	ASSERT_FALSE(cpv::isSafePath("./dir/test..txt"));
	ASSERT_FALSE(cpv::isSafePath("../dir/test.txt"));
	ASSERT_FALSE(cpv::isSafePath("./dir//test.txt"));
	ASSERT_FALSE(cpv::isSafePath("./dir\\\\test.txt"));
	ASSERT_FALSE(cpv::isSafePath("./dir\rtest.txt"));
	ASSERT_FALSE(cpv::isSafePath("./dir\ntest.txt"));
	ASSERT_FALSE(cpv::isSafePath(std::string_view("a\x00""b", 3)));
}

