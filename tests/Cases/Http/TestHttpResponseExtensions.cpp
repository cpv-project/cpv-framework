#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestHttpResponseExtensions, reply) {
	return seastar::do_with(
		cpv::HttpResponse(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& response, auto& str) {
		response.setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return cpv::extensions::reply(response, "test contents").then([&response, &str] {
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), cpv::constants::TextPlainUtf8);
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength), "13");
			ASSERT_EQ(*str, "test contents");
		});
	});
}

TEST_FUTURE(TestHttpResponseExtensions, replyWithMime) {
	return seastar::do_with(
		cpv::HttpResponse(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& response, auto& str) {
		response.setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return cpv::extensions::reply(response, "{ }", cpv::constants::ApplicationJsonUtf8)
		.then([&response, &str] {
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), cpv::constants::ApplicationJsonUtf8);
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength), "3");
			ASSERT_EQ(*str, "{ }");
		});
	});
}

TEST_FUTURE(TestHttpResponseExtensions, replyWithMimeAndStatusCode) {
	return seastar::do_with(
		cpv::HttpResponse(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& response, auto& str) {
		response.setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return cpv::extensions::reply(
			response, std::string("{} //test", 3), cpv::constants::ApplicationJsonUtf8,
			cpv::constants::_404, cpv::constants::NotFound)
			.then([&response, &str] {
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_404);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::NotFound);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), cpv::constants::ApplicationJsonUtf8);
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength), "3");
			ASSERT_EQ(*str, "{} ");
		});
	});
}

TEST_FUTURE(TestHttpResponseExtensions, redirectTo) {
	return seastar::do_with(
		cpv::HttpResponse(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& response, auto& str) {
		response.setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return cpv::extensions::redirectTo(response, "/login").then([&response, &str] {
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_302);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::Found);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::Location), "/login");
			ASSERT_EQ(*str, "");
		});
	});
}

TEST_FUTURE(TestHttpResponseExtensions, redirectToPermanently) {
	return seastar::do_with(
		cpv::HttpResponse(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& response, auto& str) {
		response.setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return cpv::extensions::redirectToPermanently(response, "/login").then([&response, &str] {
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_301);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::MovedPermanently);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::Location), "/login");
			ASSERT_EQ(*str, "");
		});
	});
}

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

