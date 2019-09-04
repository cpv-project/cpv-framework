#include <CPVFramework/Utility/CodeInfo.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestCodeInfo, construct) {
	auto codeInfo = CPV_CODEINFO;
	auto codeInfoStr = codeInfo.str();
	ASSERT_TRUE(codeInfoStr.find("TestCodeInfo.cpp") != std::string::npos);
	ASSERT_TRUE(codeInfoStr.find("construct") != std::string::npos);
	ASSERT_TRUE(codeInfoStr.find("5") != std::string::npos);
}

