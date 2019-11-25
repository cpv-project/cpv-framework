#include <seastar/core/future-util.hh>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	class TestHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext&,
			cpv::HttpServerRequestHandlerIterator) const override {
			return seastar::make_exception_future<>(cpv::FormatException(
				CPV_CODEINFO, "test exception"));
		}
	};
	
	class TestLogger : public cpv::Logger {
	public:
		TestLogger() :
			cpv::Logger(cpv::LogLevel::Error), content() { }
		
		void logImpl(cpv::LogLevel, const std::string& str) override {
			content.append(str);
		}
		
		std::string content;
	};
}

TEST_FUTURE(HttpServerRequest500Handler, handle) {
	return seastar::do_with(
		cpv::HttpServerRequestHandlerCollection(),
		cpv::HttpContext(),
		seastar::make_lw_shared<cpv::SharedStringBuilder>(),
		seastar::make_shared<TestLogger>(),
		[] (auto& handlers, auto& context, auto& str, auto& logger) {
		context.getResponse().setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest500Handler>(logger));
		handlers.emplace_back(seastar::make_shared<TestHandler>());
		return handlers.at(0)->handle(context, handlers.begin()+1).then([&context, &str, &logger] {
			auto& response = context.getResponse();
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_500);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::InternalServerError);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), cpv::constants::TextPlainUtf8);
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength),
				cpv::constants::Integers.at(str->size()));
			ASSERT_CONTAINS(str->view(), cpv::constants::InternalServerError);
			ASSERT_CONTAINS(str->view(), "ID: ");
			std::size_t index = str->view().find("ID: ");
			std::string_view id = str->view().substr(index);
			ASSERT_EQ(id.size(), 40U); // length of uuid is 36
			ASSERT_CONTAINS(logger->content, "Http server request error, ID: ");
			ASSERT_CONTAINS(logger->content, "test exception");
			ASSERT_CONTAINS(logger->content, id);
		});
	});
}

