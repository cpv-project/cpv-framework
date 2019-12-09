#include <CPVFramework/Serialize/FormSerializer.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	class MyModel {
	public:
		int intValue;
		std::size_t sizeValue;
		double doubleValue;
		std::string stringValue;
		cpv::SharedString sharedStringValue;
		std::vector<int> intValues;

		void dumpForm(cpv::HttpForm& form) const {
			form.add("intValue", cpv::SharedString::fromInt(intValue));
			form.add("sizeValue", cpv::SharedString::fromInt(sizeValue));
			form.add("doubleValue", cpv::SharedString::fromDouble(doubleValue));
			form.add("stringValue", cpv::SharedString(stringValue));
			form.add("sharedStringValue", sharedStringValue.share());
			for (int value : intValues) {
				form.add("intValues", cpv::SharedString::fromInt(value));
			}
		}

		MyModel() :
			intValue(),
			sizeValue(),
			doubleValue(),
			stringValue(),
			sharedStringValue(),
			intValues() { }
	};
}

TEST(FormSerializer, model) {
	MyModel model;
	model.intValue = 101;
	model.sizeValue = 102;
	model.doubleValue = 0.1;
	model.stringValue = "abc";
	model.sharedStringValue = "一二三";
	model.intValues = { 1, 2, 100 };
	auto packet = cpv::serializeForm(model);
	ASSERT_EQ(packet.toString(),
		"doubleValue=0.1&intValue=101&"
		"intValues=1&intValues=2&intValues=100&"
		"sharedStringValue=%E4%B8%80%E4%BA%8C%E4%B8%89&"
		"sizeValue=102&stringValue=abc");
}

TEST(FormSerializer, ptrWrappedModel) {
	{
		auto model = std::make_unique<MyModel>();
		model->intValue = 101;
		model->sizeValue = 102;
		model->doubleValue = 0.1;
		model->stringValue = "abc";
		model->sharedStringValue = "一二三";
		model->intValues = { 1, 2, 100 };
		auto packet = cpv::serializeForm(model);
		ASSERT_EQ(packet.toString(),
			"doubleValue=0.1&intValue=101&"
			"intValues=1&intValues=2&intValues=100&"
			"sharedStringValue=%E4%B8%80%E4%BA%8C%E4%B8%89&"
			"sizeValue=102&stringValue=abc");
	}
	{
		std::unique_ptr<MyModel> model;
		auto packet = cpv::serializeForm(model);
		ASSERT_EQ(packet.toString(), "");
	}
}

