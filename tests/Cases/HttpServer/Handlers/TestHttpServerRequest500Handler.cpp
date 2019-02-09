#include <seastar/core/future-util.hh>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	class TestHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpRequest&,
			cpv::HttpResponse&,
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
	return seastar::do_with(
		std::vector<std::unique_ptr<cpv::HttpServerRequestHandlerBase>>(),
		cpv::HttpRequest(),
		cpv::HttpResponse(),
		seastar::make_lw_shared<std::string>(),
		seastar::make_shared<TestLogger>(),
		[] (auto& handlers, auto& request, auto& response, auto& str, auto& logger) {
		handlers.emplace_back(std::make_unique<cpv::HttpServerRequest500Handler>(logger));
		handlers.emplace_back(std::make_unique<TestHandler>());
		response.setBodyStream(
			cpv::makeObject<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return handlers.at(0)->handle(request, response, handlers.begin()+1).then(
			[&response, &str, &logger] {
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_500);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::InternalServerError);
			ASSERT_EQ(response.getHeaders().at(cpv::constants::ContentType),
				cpv::constants::TextPlainUtf8);
			ASSERT_EQ(response.getHeaders().at(cpv::constants::ContentLength),
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

