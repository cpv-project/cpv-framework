#include <CPVFramework/Utility/JsonUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	struct A {
		std::string name;
		std::size_t age;
	};

	struct B {
		std::string name;
		std::size_t age;

		void toJson(cpv::Json& json) const {
			json["name"] = name;
			json["age"] = age;
		}

		void fromJson(const cpv::Json& json) {
			name = json.at("name");
			age = json.at("age");
		}
	};
}

namespace cpv {
	template <>
	struct JsonSerializer<A> {
		static void toJson(Json& json, const A& value) {
			json["name"] = value.name;
			json["age"] = value.age;
		}

		static void fromJson(const Json& json, A& value) {
			value.name = json.at("name");
			value.age = json.at("age");
		}
	};
}

TEST(TestJsonUtils, customSerialize) {
	{
		cpv::Json serialize = A({ "abc", 123 });
		std::string str = serialize.dump();

		auto deserialize = cpv::Json::parse(str);
		auto deserialized = deserialize.get<A>();
		ASSERT_EQ(deserialized.name, "abc");
		ASSERT_EQ(deserialized.age, 123);
	}
	{
		cpv::Json serialize;
		serialize["key"] = B({ "abc", 123 });
		std::string str = serialize.dump();

		auto deserialize = cpv::Json::parse(str);
		auto deserialized = deserialize["key"].get<B>();
		ASSERT_EQ(deserialized.name, "abc");
		ASSERT_EQ(deserialized.age, 123);
	}
}

