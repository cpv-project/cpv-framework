#include <CPVFramework/Serialize/JsonSerializer.hpp>
#include <CPVFramework/Serialize/JsonDeserializer.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	class MyModel {
	public:
		class ChildModel {
		public:
			int count = 0;

			void dumpJson(cpv::JsonBuilder& builder) const {
				builder.startObject()
					.addMember(CPV_JSONKEY("count"), count)
					.endObject();
			}
		};

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

		void dumpJson(cpv::JsonBuilder& builder) const {
			builder.startObject()
				.addMember(CPV_JSONKEY("intValue"), intValue)
				.addMember(CPV_JSONKEY("sizeValue"), sizeValue)
				.addMember(CPV_JSONKEY("doubleValue"), doubleValue)
				.addMember(CPV_JSONKEY("durationValue"), durationValue)
				.addMember(CPV_JSONKEY("stringValue"), stringValue)
				.addMember(CPV_JSONKEY("sharedStringValue"), sharedStringValue)
				.addMember(CPV_JSONKEY("childValue"), childValue)
				.addMember(CPV_JSONKEY("childValues"), childValues)
				.addMember(CPV_JSONKEY("intValues"), intValues)
				.addMember(CPV_JSONKEY("intValuesOnStack"), intValuesOnStack)
				.endObject();
		}

		MyModel() :
			intValue(),
			sizeValue(),
			doubleValue(),
			durationValue(),
			stringValue(),
			sharedStringValue(),
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

			void reset(int countVal) {
				count = countVal;
			}

			void dumpJson(cpv::JsonBuilder& builder) const {
				builder.startObject().addMember("count", count).endObject();
			}
		};

		std::optional<int> optionalValue;
		std::unique_ptr<cpv::SharedString> uniquePtrValue;
		seastar::shared_ptr<std::vector<int>> sharedPtrValue;
		cpv::Reusable<ChildModel> reusableValue;

		void dumpJson(cpv::JsonBuilder& builder) const {
			builder.startObject()
				.addMember("optionalValue", optionalValue)
				.addMember("uniquePtrValue", uniquePtrValue)
				.addMember("sharedPtrValue", sharedPtrValue)
				.addMember("reusableValue", reusableValue)
				.endObject();
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

TEST(JsonSerializer, model) {
	MyModel model;
	model.intValue = 101;
	model.sizeValue = 102;
	model.doubleValue = 103.1;
	model.durationValue = std::chrono::seconds(321);
	model.stringValue = "test\n一二三";
	model.sharedStringValue = "一二三\"";
	model.childValue.count = -1;
	model.childValues.resize(3);
	model.childValues.at(0).count = 1;
	model.childValues.at(1).count = 2;
	model.childValues.at(2).count = 3;
	model.intValues = { 1, 2, 3, 4, 5 };
	model.intValuesOnStack = { -1, -2, -3  };
	cpv::Packet packet = cpv::serializeJson(model);
	cpv::SharedString json = packet.toString();
	ASSERT_EQ(json,
		"{\"intValue\":101,\"sizeValue\":102,\"doubleValue\":103.1,"
		"\"durationValue\":321,"
		"\"stringValue\":\"test\\n\xE4\xB8\x80\xE4\xBA\x8C\xE4\xB8\x89\","
		"\"sharedStringValue\":\"\xE4\xB8\x80\xE4\xBA\x8C\xE4\xB8\x89\\\"\","
		"\"childValue\":{\"count\":-1},"
		"\"childValues\":[{\"count\":1},{\"count\":2},{\"count\":3}],"
		"\"intValues\":[1,2,3,4,5],"
		"\"intValuesOnStack\":[-1,-2,-3]}");
	std::optional<cpv::JsonDocument> document;
	auto error = cpv::deserializeJson(document, json);
	ASSERT_FALSE(error.has_value());
}

TEST(JsonSerializer, vectorModel) {
	std::vector<MyModel> models;
	models.resize(3);
	for (std::size_t i = 0; i < models.size(); ++i) {
		auto& model = models[i];
		model.intValue = 100 + i;
	}
	cpv::Packet packet = cpv::serializeJson(models);
	cpv::SharedString json = packet.toString();
	ASSERT_EQ(json,
		"["
		"{\"intValue\":100,\"sizeValue\":0,\"doubleValue\":0,\"durationValue\":0,"
		"\"stringValue\":\"\",\"sharedStringValue\":\"\","
		"\"childValue\":{\"count\":0},\"childValues\":[],"
		"\"intValues\":[],\"intValuesOnStack\":[]},"
		"{\"intValue\":101,\"sizeValue\":0,\"doubleValue\":0,\"durationValue\":0,"
		"\"stringValue\":\"\",\"sharedStringValue\":\"\","
		"\"childValue\":{\"count\":0},\"childValues\":[],"
		"\"intValues\":[],\"intValuesOnStack\":[]},"
		"{\"intValue\":102,\"sizeValue\":0,\"doubleValue\":0,\"durationValue\":0,"
		"\"stringValue\":\"\",\"sharedStringValue\":\"\","
		"\"childValue\":{\"count\":0},\"childValues\":[],"
		"\"intValues\":[],\"intValuesOnStack\":[]}"
		"]");
	std::optional<cpv::JsonDocument> document;
	auto error = cpv::deserializeJson(document, json);
	ASSERT_FALSE(error.has_value());
}

TEST(JsonSerializer, stackAllocatedVectorModel) {
	cpv::StackAllocatedVector<MyModel, 2> models;
	models.resize(2);
	for (std::size_t i = 0; i < models.size(); ++i) {
		auto& model = models[i];
		model.intValue = -100 - i;
	}
	cpv::Packet packet = cpv::serializeJson(models);
	cpv::SharedString json = packet.toString();
	ASSERT_EQ(json,
		"["
		"{\"intValue\":-100,\"sizeValue\":0,\"doubleValue\":0,\"durationValue\":0,"
		"\"stringValue\":\"\",\"sharedStringValue\":\"\","
		"\"childValue\":{\"count\":0},\"childValues\":[],"
		"\"intValues\":[],\"intValuesOnStack\":[]},"
		"{\"intValue\":-101,\"sizeValue\":0,\"doubleValue\":0,\"durationValue\":0,"
		"\"stringValue\":\"\",\"sharedStringValue\":\"\","
		"\"childValue\":{\"count\":0},\"childValues\":[],"
		"\"intValues\":[],\"intValuesOnStack\":[]}"
		"]");
	std::optional<cpv::JsonDocument> document;
	auto error = cpv::deserializeJson(document, json);
	ASSERT_FALSE(error.has_value());
}

TEST(JsonSerializer, ptrModel) {
	{
		MyPtrModel model;
		cpv::Packet packet = cpv::serializeJson(model);
		cpv::SharedString json = packet.toString();
		ASSERT_EQ(json,
			"{\"optionalValue\":null,\"uniquePtrValue\":null,"
			"\"sharedPtrValue\":null,\"reusableValue\":null}");
		std::optional<cpv::JsonDocument> document;
		auto error = cpv::deserializeJson(document, json);
		ASSERT_FALSE(error.has_value());
	}
	{
		MyPtrModel model;
		model.optionalValue = 123;
		model.uniquePtrValue = std::make_unique<cpv::SharedString>("test\"");
		model.sharedPtrValue = seastar::make_shared<std::vector<int>>(
			std::vector<int>({ 1, 2, 3 }));
		model.reusableValue = cpv::makeReusable<MyPtrModel::ChildModel>(321);
		cpv::Packet packet = cpv::serializeJson(model);
		cpv::SharedString json = packet.toString();
		ASSERT_EQ(json,
			"{\"optionalValue\":123,\"uniquePtrValue\":\"test\\\"\","
			"\"sharedPtrValue\":[1,2,3],\"reusableValue\":{\"count\":321}}");
		std::optional<cpv::JsonDocument> document;
		auto error = cpv::deserializeJson(document, json);
		ASSERT_FALSE(error.has_value());
	}
}

TEST(JsonSerializer, ptrWrappedModel) {
	{
		auto model = std::make_unique<MyModel>();
		model->intValue = 101;
		cpv::Packet packet = cpv::serializeJson(model);
		cpv::SharedString json = packet.toString();
		ASSERT_EQ(json,
			"{\"intValue\":101,\"sizeValue\":0,\"doubleValue\":0,\"durationValue\":0,"
			"\"stringValue\":\"\",\"sharedStringValue\":\"\","
			"\"childValue\":{\"count\":0},\"childValues\":[],"
			"\"intValues\":[],\"intValuesOnStack\":[]}");
	}
	{
		std::unique_ptr<MyModel> model;
		cpv::Packet packet = cpv::serializeJson(model);
		cpv::SharedString json = packet.toString();
		ASSERT_EQ(json, "null");
	}
}

