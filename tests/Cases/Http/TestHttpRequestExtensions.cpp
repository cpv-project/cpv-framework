#include <CPVFramework/Http/HttpRequestExtensions.hpp>
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

TEST_FUTURE(HttpRequestExtensions, readBodyStream) {
	return seastar::do_with(
		cpv::HttpRequest(),
		std::string(),
		[] (auto& request, auto& source) {
		for (std::size_t i = 0; i < 9000; ++i) {
			source.append(1, static_cast<char>(i));
		}
		request.setBodyStream(cpv::makeReusable<cpv::StringInputStream>(
			cpv::SharedString(source)).cast<cpv::InputStreamBase>());
		return cpv::extensions::readBodyStream(request).then([&source] (cpv::SharedString str) {
			ASSERT_EQ(str, source);
		}).then([&request] {
			request.setBodyStream(cpv::Reusable<cpv::InputStreamBase>());
			return cpv::extensions::readBodyStream(request);
		}).then([] (cpv::SharedString str) {
			ASSERT_TRUE(str.empty());
		});
	});
}

TEST_FUTURE(HttpRequestExtensions, readBodyStreamAsJson) {
	return seastar::do_with(
		cpv::HttpRequest(),
		MyModel(),
		[] (auto& request, auto& model) {
		cpv::SharedString json("{ intValue: 123 }");
		request.setBodyStream(
			cpv::makeReusable<cpv::StringInputStream>(std::move(json))
			.cast<cpv::InputStreamBase>());
		return cpv::extensions::readBodyStreamAsJson(request, model).then([&model] {
			ASSERT_EQ(model.intValue, 123);
		}).then([&request, &model] {
			request.setBodyStream(cpv::Reusable<cpv::InputStreamBase>());
			return cpv::extensions::readBodyStreamAsJson(request, model);
		}).then_wrapped([] (seastar::future<> f) {
			ASSERT_THROWS(cpv::DeserializeException, f.get());
		});
	});
}

TEST_FUTURE(HttpRequestExtensions, readBodyStreamAsForm) {
	return seastar::do_with(
		cpv::HttpRequest(),
		MyModel(),
		[] (auto& request, auto& model) {
		cpv::SharedString formStr("intValue=123");
		request.setBodyStream(
			cpv::makeReusable<cpv::StringInputStream>(std::move(formStr))
			.cast<cpv::InputStreamBase>());
		return cpv::extensions::readBodyStreamAsForm(request, model).then([&model] {
			ASSERT_EQ(model.intValue, 123);
		}).then([&request, &model] {
			request.setBodyStream(cpv::Reusable<cpv::InputStreamBase>());
			return cpv::extensions::readBodyStreamAsForm(request, model);
		}).then([&model] {
			ASSERT_EQ(model.intValue, 0);
		});
	});
}

