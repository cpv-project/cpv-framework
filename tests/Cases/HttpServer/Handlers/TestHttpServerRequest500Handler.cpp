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
			const cpv::HttpServerRequestHandlerIterator&) const override {
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
	auto logger = seastar::make_shared<TestLogger>();
	cpv::Container container;
	container.add<seastar::shared_ptr<cpv::Logger>>(logger);
	return seastar::do_with(
		cpv::HttpServerRequestHandlerCollection(),
		cpv::HttpContext(container),
		seastar::make_lw_shared<std::string>(),
		std::move(logger),
		[] (auto& handlers, auto& context, auto& str, auto& logger) {
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest500Handler>());
		handlers.emplace_back(seastar::make_shared<TestHandler>());
		context.response.setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return handlers.at(0)->handle(context, handlers.begin()+1).then([&context, &str, &logger] {
			auto& response = context.response;
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_500);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::InternalServerError);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), cpv::constants::TextPlainUtf8);
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength),
				cpv::constants::Integers.at(str->size()));
			ASSERT_CONTAINS(*str, cpv::constants::InternalServerError);
			ASSERT_CONTAINS(*str, "ID: ");
			std::size_t index = str->find("ID: ");
			std::string id = str->substr(index);
			ASSERT_EQ(id.size(), 40U); // length of uuid is 36
			ASSERT_CONTAINS(logger->content, "Http server request error, ID: ");
			ASSERT_CONTAINS(logger->content, "test exception");
			ASSERT_CONTAINS(logger->content, id);
		});
	});
}

