#include <CPVFramework/Exceptions/Exception.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestException, construct) {
	cpv::Exception exception(CPV_CODEINFO, "some error");
	std::string error = exception.what();
	// std::cout << error << std::endl;
	ASSERT_TRUE(error.find("some error") != std::string::npos);
	ASSERT_TRUE(error.find("TestException.cpp") != std::string::npos);
}

