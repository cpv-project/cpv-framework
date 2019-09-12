#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestHttpResponseExtensions, setCookie) {
	cpv::HttpResponse response;
	cpv::extensions::setCookie(response, "key", "value");
	cpv::extensions::setCookie(response, "other", "variable", "");
	cpv::extensions::setCookie(response, "novalue", "", "");
	cpv::extensions::setCookie(response, "key", "value", "/path", "do.main");
	cpv::extensions::setCookie(response, "expires", "", "", "", 1568771516);
	cpv::extensions::setCookie(response, "secure", "cookie", "", "", {}, false, true, "Strict");
	cpv::extensions::setCookie(response, "httponly", "cookie", "", "", {}, true, false, "Lax");
	auto& additionHeaders = response.getHeaders().getAdditionHeaders();
	ASSERT_EQ(additionHeaders.size(), 7U);
	ASSERT_EQ(additionHeaders.at(0).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(0).second, "key=value; Path=/");
	ASSERT_EQ(additionHeaders.at(1).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(1).second, "other=variable");
	ASSERT_EQ(additionHeaders.at(2).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(2).second, "novalue");
	ASSERT_EQ(additionHeaders.at(3).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(3).second, "key=value; Path=/path; Domain=do.main");
	ASSERT_EQ(additionHeaders.at(4).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(4).second, "expires; Expires=Wed, 18 Sep 2019 01:51:56 GMT");
	ASSERT_EQ(additionHeaders.at(5).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(5).second, "secure=cookie; Secure; SameSite=Strict");
	ASSERT_EQ(additionHeaders.at(6).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(6).second, "httponly=cookie; HttpOnly; SameSite=Lax");
}

TEST(TestHttpResponseExtensions, removeCookie) {
	cpv::HttpResponse response;
	cpv::extensions::removeCookie(response, "key");
	cpv::extensions::removeCookie(response, "otherKey", "/path");
	cpv::extensions::removeCookie(response, "anotherKey", "/path", "do.main");
	auto& additionHeaders = response.getHeaders().getAdditionHeaders();
	ASSERT_EQ(additionHeaders.size(), 3U);
	ASSERT_EQ(additionHeaders.at(0).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(0).second,
		"key; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
	ASSERT_EQ(additionHeaders.at(1).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(1).second,
		"otherKey; Path=/path; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
	ASSERT_EQ(additionHeaders.at(2).first, cpv::constants::SetCookie);
	ASSERT_EQ(additionHeaders.at(2).second,
		"anotherKey; Path=/path; Domain=do.main; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
}

