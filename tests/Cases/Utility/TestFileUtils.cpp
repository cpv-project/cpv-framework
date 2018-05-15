#include <unistd.h>
#include <CPVFramework/Utility/FileUtils.hpp>
#include <CPVFramework/Exceptions/FileSystemException.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	static const std::string path("/tmp/cpv-framework-test-file-utils.txt");
}

TEST(TestFileUtils, readFile) {
	cpv::writeFile(path, "test contents");
	ASSERT_EQ(cpv::readFile(path), "test contents");
	::unlink(path.c_str());
}

TEST(TestFileUtils, readFileError) {
	::unlink(path.c_str());
	ASSERT_THROWS(
		cpv::FileSystemException,
		cpv::readFile(path));
}

TEST(TestFileUtils, writeFile) {
	cpv::writeFile(path, "test contents");
	ASSERT_EQ(cpv::readFile(path), "test contents");
	::unlink(path.c_str());
}

