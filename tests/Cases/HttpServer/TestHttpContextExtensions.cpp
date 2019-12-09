#include <CPVFramework/HttpServer/HttpContextExtensions.hpp>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	class MyModel {
	public:
		int intValue;

		bool loadJson(const cpv::JsonValue& value) {
			intValue << value["intValue"];
			return true;
		}

		void loadForm(const cpv::HttpForm& form) {
			intValue = form.get("intValue").toInt().value_or(0);
		}
	};
}

TEST(HttpContextExtensions, getParameter_PathFragment) {
	using namespace cpv::extensions::http_context_parameters;
	cpv::HttpContext context;
	context.getRequest().setUrl("/test/abc/123");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(0)), "test");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(1)), "abc");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(2)), "123");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(3)), "");
}

TEST(HttpContextExtensions, getParameter_Query) {
	using namespace cpv::extensions::http_context_parameters;
	cpv::HttpContext context;
	context.getRequest().setUrl("/test?key=123&value=321");
	ASSERT_EQ(cpv::extensions::getParameter(context, Query("key")), "123");
	ASSERT_EQ(cpv::extensions::getParameter(context, Query("value")), "321");
	ASSERT_EQ(cpv::extensions::getParameter(context, Query("non-exists")), "");
}

TEST(HttpContextExtensions, getParameter_Service) {
	using namespace cpv::extensions::http_context_parameters;
	cpv::Container container;
	container.add<int>(1);
	container.add<std::string>("a");
	container.add<std::string>("b");
	cpv::HttpContext context;
	context.setContainer(container);
	ASSERT_EQ(cpv::extensions::getParameter(context, Service<int>()), 1);
	{
		auto values = cpv::extensions::getParameter(
			context, Service<std::vector<std::string>>());
		ASSERT_EQ(values.size(), 2U);
		ASSERT_EQ(values.at(0), "a");
		ASSERT_EQ(values.at(1), "b");
	}
	ASSERT_EQ(cpv::extensions::getParameter(
		context, Service<std::optional<int>>()).value_or(0), 1);
	ASSERT_FALSE(cpv::extensions::getParameter(
		context, Service<std::optional<double>>()).has_value());
}

TEST(HttpContextExtensions, getParameter_JsonModel) {
	cpv::HttpContext context;
	/*cpv::SharedString json("{ intValue: 123 }");
	request.setBodyStream(
		cpv::makeReusable<cpv::StringInputStream>(std::move(json))
		.cast<cpv::InputStreamBase>());*/
}

TEST(HttpContextExtensions, getParameter_FormModel) {
	
}

