#include <CPVFramework/Serialize/JsonDeserializer.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	class MyModel {
	public:
		class ChildModel {
		public:
			int count = 0;

			bool loadJson(const cpv::JsonValue& value) {
				count << value["count"];
				return true;
			}
		};

		int requiredValue;
		int intValue;
		std::size_t sizeValue;
		double doubleValue;
		std::chrono::seconds durationValue;
		std::string stringValue;
		cpv::SharedString sharedStringValue;
		ChildModel childValue;
		std::vector<ChildModel> childValues;
		std::vector<int> intValues;
		cpv::StackAllocatedVector<int, 3> intValuesOnStack;

		bool loadJson(const cpv::JsonValue& value) {
			if (!cpv::convertJsonValue(requiredValue, value["requiredValue"])) {
				return false;
			}
			intValue << value["intValue"];
			sizeValue << value["sizeValue"];
			doubleValue << value["doubleValue"];
			durationValue << value["durationValue"];
			stringValue << value["stringValue"];
			sharedStringValue << value["sharedStringValue"];
			childValue << value["childValue"];
			childValues << value["childValues"];
			intValues << value["intValues"];
			intValuesOnStack << value["intValuesOnStack"];
			return true;
		}

		MyModel() :
			requiredValue(-1),
			intValue(-1),
			sizeValue(-1),
			doubleValue(-1),
			durationValue(-1),
			stringValue("default"),
			sharedStringValue("default"),
			childValue(),
			childValues(),
			intValues(),
			intValuesOnStack() { }
	};

	class MyPtrModel {
	public:
		struct ChildModel {
			int count = 0;

			static void freeResources() { }

			static void reset() { }

			bool loadJson(const cpv::JsonValue& value) {
				count << value["count"];
				return true;
			}
		};

		std::optional<int> optionalValue;
		std::unique_ptr<cpv::SharedString> uniquePtrValue;
		seastar::shared_ptr<std::vector<int>> sharedPtrValue;
		cpv::Reusable<ChildModel> reusableValue;

		bool loadJson(const cpv::JsonValue& value) {
			optionalValue << value["optionalValue"];
			uniquePtrValue << value["uniquePtrValue"];
			sharedPtrValue << value["sharedPtrValue"];
			reusableValue << value["reusableValue"];
			return true;
		}

		MyPtrModel() :
			optionalValue(),
			uniquePtrValue(),
			sharedPtrValue(),
			reusableValue() { }
	};
}

template <>
thread_local cpv::ReusableStorageType<MyPtrModel::ChildModel>
	cpv::ReusableStorageInstance<MyPtrModel::ChildModel>;

TEST(JsonDeserializer, model) {
	cpv::SharedString json(std::string_view(R"(
		{
			"requiredValue": 100,
			"intValue": 101,
			"sizeValue": 102,
			"doubleValue": 103,
			"durationValue": 321,
			"stringValue": "test 一二三",
			"sharedStringValue": "\u4e00\u4e8c\u4e09",
			"childValue": { "count": -1 },
			"childValues": [
				{ "count": 1 },
				{ "count": 2 },
				{ }
			],
			"intValues": [ 1, 2, 3, 4, 5 ],
			"intValuesOnStack": [ -1, -2, -3 ]
		}
	)"));
	MyModel model;
	auto error = cpv::deserializeJson(model, json);
	ASSERT_FALSE(error.has_value());
	ASSERT_EQ(model.requiredValue, 100);
	ASSERT_EQ(model.intValue, 101);
	ASSERT_EQ(model.sizeValue, 102U);
	ASSERT_EQ((int)model.doubleValue, 103);
	ASSERT_EQ(model.durationValue.count(), 321U);
	ASSERT_EQ(model.stringValue, "test 一二三");
	ASSERT_EQ(model.sharedStringValue, "一二三");
	ASSERT_EQ(model.childValue.count, -1);
	ASSERT_EQ(model.childValues.size(), 3U);
	ASSERT_EQ(model.childValues.at(0).count, 1);
	ASSERT_EQ(model.childValues.at(1).count, 2);
	ASSERT_EQ(model.childValues.at(2).count, 0);
	ASSERT_EQ(model.intValues.size(), 5U);
	ASSERT_EQ(model.intValues.at(0), 1);
	ASSERT_EQ(model.intValues.at(1), 2);
	ASSERT_EQ(model.intValues.at(2), 3);
	ASSERT_EQ(model.intValues.at(3), 4);
	ASSERT_EQ(model.intValues.at(4), 5);
	ASSERT_EQ(model.intValuesOnStack.size(), 3U);
	ASSERT_EQ(model.intValuesOnStack.at(0), -1);
	ASSERT_EQ(model.intValuesOnStack.at(1), -2);
	ASSERT_EQ(model.intValuesOnStack.at(2), -3);
}

TEST(JsonDeserializer, vectorModel) {
	cpv::SharedString json(std::string_view(R"(
		[
			{ "requiredValue": 100 },
			{ "requiredValue": 101 },
			{ "requiredValue": 102 }
		]
	)"));
	std::vector<MyModel> models;
	auto error = cpv::deserializeJson(models, json);
	ASSERT_FALSE(error.has_value());
	ASSERT_EQ(models.size(), 3U);
	ASSERT_EQ(models.at(0).requiredValue, 100);
	ASSERT_EQ(models.at(1).requiredValue, 101);
	ASSERT_EQ(models.at(2).requiredValue, 102);
}

TEST(JsonDeserializer, stackAllocatedVectorModel) {
	cpv::SharedString json(std::string_view(R"(
		[
			{ "requiredValue": 100 },
			{ "requiredValue": 101 },
			{ "requiredValue": 102 }
		]
	)"));
	cpv::StackAllocatedVector<MyModel, 3> models;
	auto error = cpv::deserializeJson(models, json);
	ASSERT_FALSE(error.has_value());
	ASSERT_EQ(models.size(), 3U);
	ASSERT_EQ(models.at(0).requiredValue, 100);
	ASSERT_EQ(models.at(1).requiredValue, 101);
	ASSERT_EQ(models.at(2).requiredValue, 102);
}

TEST(JsonDeserializer, ptrModel) {
	MyPtrModel model;
	{
		cpv::SharedString json(std::string_view(R"(
			{
				"optionalValue": 123,
				"uniquePtrValue": "test",
				"sharedPtrValue": [ 1, 2, 3 ],
				"reusableValue": { "count": 321 }
			}
		)"));
		auto error = cpv::deserializeJson(model, json);
		json = {};
		ASSERT_FALSE(error.has_value());
		ASSERT_TRUE(model.optionalValue.has_value());
		ASSERT_EQ(*model.optionalValue, 123);
		ASSERT_TRUE(model.uniquePtrValue != nullptr);
		ASSERT_EQ(*model.uniquePtrValue, "test");
		ASSERT_TRUE(model.sharedPtrValue != nullptr);
		ASSERT_EQ(model.sharedPtrValue->size(), 3U);
		ASSERT_EQ(model.sharedPtrValue->at(0), 1);
		ASSERT_EQ(model.sharedPtrValue->at(1), 2);
		ASSERT_EQ(model.sharedPtrValue->at(2), 3);
		ASSERT_TRUE(model.reusableValue != nullptr);
		ASSERT_EQ(model.reusableValue->count, 321);
	}
	{
		cpv::SharedString json(std::string_view("{}"));
		auto error = cpv::deserializeJson(model, json);
		json = {};
		ASSERT_FALSE(error.has_value());
		ASSERT_TRUE(model.optionalValue.has_value());
		ASSERT_EQ(*model.optionalValue, 123);
		ASSERT_TRUE(model.uniquePtrValue != nullptr);
		ASSERT_EQ(*model.uniquePtrValue, "test");
		ASSERT_TRUE(model.sharedPtrValue != nullptr);
		ASSERT_EQ(model.sharedPtrValue->size(), 3U);
		ASSERT_EQ(model.sharedPtrValue->at(0), 1);
		ASSERT_EQ(model.sharedPtrValue->at(1), 2);
		ASSERT_EQ(model.sharedPtrValue->at(2), 3);
		ASSERT_TRUE(model.reusableValue != nullptr);
		ASSERT_EQ(model.reusableValue->count, 321);
	}
	{
		cpv::SharedString json(std::string_view(R"(
			{
				"optionalValue": null,
				"uniquePtrValue": null,
				"sharedPtrValue": null,
				"reusableValue": null
			}
		)"));
		auto error = cpv::deserializeJson(model, json);
		json = {};
		ASSERT_FALSE(error.has_value());
		ASSERT_FALSE(model.optionalValue.has_value());
		ASSERT_FALSE(model.uniquePtrValue != nullptr);
		ASSERT_FALSE(model.sharedPtrValue != nullptr);
		ASSERT_FALSE(model.reusableValue != nullptr);
	}
}

TEST(JsonDeserializer, ptrWrappedModel) {
	cpv::SharedString json(std::string_view(R"(
		{
			"requiredValue": 100
		}
	)"));
	std::unique_ptr<MyModel> model;
	auto error = cpv::deserializeJson(model, json);
	ASSERT_FALSE(error.has_value());
	ASSERT_TRUE(model != nullptr);
	ASSERT_EQ(model->requiredValue, 100);
}

TEST(JsonDeserializer, modelWithEmptyJson) {
	cpv::SharedString json;
	MyModel model;
	auto error = cpv::deserializeJson(model, json);
	ASSERT_TRUE(error.has_value());
	ASSERT_CONTAINS(std::string_view(error->what()), "missing root element");
}

TEST(JsonDeserializer, modelWithNullJson) {
	cpv::SharedString json("null");
	std::unique_ptr<MyModel> model;
	auto error = cpv::deserializeJson(model, json);
	ASSERT_TRUE(error.has_value());
	ASSERT_CONTAINS(std::string_view(error->what()), "document root must be object or array");
}

TEST(JsonDeserializer, modelWithNotContainsRequiredValueJson) {
	cpv::SharedString json(std::string_view(R"(
		{
		}
	)"));
	MyModel model;
	auto error = cpv::deserializeJson(model, json);
	ASSERT_TRUE(error.has_value());
	ASSERT_CONTAINS(std::string_view(error->what()), "convert failed");
}

TEST(JsonDeserializer, modelWithCorruptedJson) {
	cpv::SharedString json(std::string_view(R"(
		{
	)"));
	MyModel model;
	auto error = cpv::deserializeJson(model, json);
	ASSERT_TRUE(error.has_value());
	ASSERT_CONTAINS(std::string_view(error->what()), "unexpected end of input");
}

TEST(JsonDeserializer, modelWithTypeUnmatchedJson) {
	cpv::SharedString json(std::string_view(R"(
		{
			"requiredValue": "1"
		}
	)"));
	MyModel model;
	auto error = cpv::deserializeJson(model, json);
	ASSERT_TRUE(error.has_value());
	ASSERT_CONTAINS(std::string_view(error->what()), "convert failed");
}

TEST(JsonDeserializer, jsonDocument) {
	cpv::SharedString json(std::string_view(R"(
		{
			"test-key": "test-value"
		}
	)"));
	std::optional<cpv::JsonDocument> document;
	auto error = cpv::deserializeJson(document, json);
	ASSERT_FALSE(error.has_value());
	ASSERT_TRUE(document.has_value());
	std::string value;
	ASSERT_TRUE(cpv::convertJsonValue(value,
		cpv::JsonValue(document->get_root(), json)["test-key"]));
	ASSERT_EQ(value, "test-value");
	ASSERT_FALSE(cpv::convertJsonValue(value,
		cpv::JsonValue(document->get_root(), json)["not-exists"]));
	ASSERT_EQ(value, "test-value");
}

TEST(JsonDeserializer, jsonDocumentWithCorruptedJson) {
	cpv::SharedString json(std::string_view(R"(
		{
	)"));
	std::optional<cpv::JsonDocument> document;
	auto error = cpv::deserializeJson(document, json);
	ASSERT_TRUE(error.has_value());
	ASSERT_FALSE(document.has_value());
	ASSERT_CONTAINS(std::string_view(error->what()), "unexpected end of input");
}

