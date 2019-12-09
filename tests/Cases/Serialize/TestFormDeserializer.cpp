#include <CPVFramework/Serialize/FormDeserializer.hpp>
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

		void loadForm(const cpv::HttpForm& form) {
			intValue = form.get("intValue").toInt().value_or(0);
			sizeValue = form.get("sizeValue").toInt<std::size_t>().value_or(0);
			doubleValue = form.get("doubleValue").toDouble().value_or(0);
			stringValue = form.get("stringValue").view();
			sharedStringValue = form.get("sharedStringValue").share();
			for (const auto& value : form.getMany("intValues")) {
				intValues.emplace_back(value.toInt().value_or(0));
			}
		}

		MyModel() :
			intValue(-1),
			sizeValue(-1),
			doubleValue(-1),
			stringValue("default"),
			sharedStringValue("default"),
			intValues() { }
	};
}

TEST(FormDeserializer, model) {
	MyModel model;
	{
		cpv::SharedString formBody(std::string_view(
			"intValue=101&sizeValue=102&doubleValue=0.1&stringValue=abc&"
			"sharedStringValue=%E4%B8%80%E4%BA%8C%E4%B8%89&"
			"intValues=1&intValues=2&intValues=100"
		));
		cpv::deserializeForm(model, formBody);
	}
	ASSERT_EQ(model.intValue, 101);
	ASSERT_EQ(model.sizeValue, 102U);
	ASSERT_EQ(model.doubleValue, 0.1);
	ASSERT_EQ(model.stringValue, "abc");
	ASSERT_EQ(model.sharedStringValue, "一二三");
	ASSERT_EQ(model.intValues.size(), 3U);
	ASSERT_EQ(model.intValues.at(0), 1);
	ASSERT_EQ(model.intValues.at(1), 2);
	ASSERT_EQ(model.intValues.at(2), 100);
}

TEST(FormDeserializer, ptrWrappedModel) {
	std::unique_ptr<MyModel> model;
	cpv::SharedString formBody("intValue=101");
	cpv::deserializeForm(model, formBody);
	ASSERT_TRUE(model != nullptr);
	ASSERT_EQ(model->intValue, 101);
}

