# Serialization

cpv framework provides data serialization and deserialization support, here are supported serializers and deserializers for now:

- [JsonSerializer](../include/CPVFramework/Serialize/JsonSerializer.hpp)
- [JsonDeserializer](../include/CPVFramework/Serialize/JsonDeserializer.hpp)
- [FormSerializer](../include/CPVFramework/Serialize/FormSerializer.hpp)
- [FormDeserializer](../include/CPVFramework/Serialize/FormDeserializer.hpp)

## Json

### JsonSerializer

The [JsonSerializer](../include/CPVFramework/Serialize/JsonSerializer.hpp) is based on `JsonBuilder` provided by cpv framework, here is an example of `JsonBuilder`:

``` c++
#include <CPVFramework/Serialize/JsonSerializer.hpp>

void example() {
	cpv::JsonBuilder builder;
	builder.startObject()
		.addMember(CPV_JSONKEY("id"), 1)
		.addMember(CPV_JSONKEY("name"), "abc")
		.endObject();
	cpv::Packet packet = builder.toPacket();
	// packet now contains {"id":1,"name":"abc"}
}
```

If you add `dumpJson` function that takes `cpv::JsonBuilder&` to your model type, then you can use `cpv::serializeJson(model)` to get the json packet, for example:

``` c++
#include <CPVFramework/Serialize/JsonSerializer.hpp>

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

void example() {
	MyModel model;
	cpv::Packet packet = cpv::serializeJson(model);
	cpv::SharedString json = packet.toString();
}
```

For performance reason, the json builder won't validate the sequence of operations, you should use unit tests to ensure the generated json structure is valid.

### JsonDeserializer

The [JsonDeserializer](../include/CPVFramework/Serialize/JsonDeserializer.hpp) is based on [sajson](https://github.com/chadaustin/sajson) library, the type `cpv::JsonType` is an alias of `sajson::type`, the type `cpv::JsonDocument` is an alias of `sajson::document`, and the type `cpv::JsonValue` is a child type of `sajson::value` with `SharedString` support.

There two way to use JsonDeserializer, one is deserialize to `cpv::JsonDocument` and handle the document, second is deseiralize to your model type which provides a `loadJson` function, the second way is more recommended.

**Warning: sajson is an in-situ json parser, which mean the input buffer must be mutable.**

Here is an example of deserialize to `cpv::JsonDocument`:

``` c++
#include <CPVFramework/Serialize/JsonDeserializer.hpp>

void example() {
	// use the copy overload of constructor to make sure the input buffer is mutable
	cpv::SharedString json(std::string_view("{\"test-key\": \"test-value\"}"));
	std::optional<cpv::JsonDocument> document;
	std::optional<cpv::DeserializeException> error = cpv::deserializeJson(document, json);
	if (error.has_value()) {
		throw *error;
	}
	cpv::JsonValue root(document->get_root(), json);
	cpv::SharedString value;
	value << root["test-key"];
	// value now contains "test-value"
}
```

Here is an example of deserialize to custom model:

``` c++
#include <CPVFramework/Serialize/JsonDeserializer.hpp>

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
		// return false if key not exists or type not matched
		// the json deserializer will report the error
		if (!cpv::convertJsonValue(requiredValue, value["requiredValue"])) {
			return false;
		}
		// operator << will only change the target if key exists and type matched
		// otherwise the target is untouched, it's useful for patch style updating
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
		// return true to indicates loaded successfully
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

void example() {
	// use the copy overload of constructor to make sure the input buffer is mutable
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
	std::optional<cpv::DeserializeException> error = cpv::deserializeJson(model, json);
	if (error.has_value()) {
		throw *error;
	}
}
```

## Form

### FormSerializer

The [FormSerializer](../include/CPVFramework/Serialize/FormSerializer.hpp) is based on [HttpForm](../include/CPVFramework/Http/HttpForm.hpp) provided by cpv framework, here is an example of `HttpForm`:

``` c++
#include <CPVFramework/Http/HttpForm.hpp>

void example() {
	cpv::HttpForm form;
	form.add("key 1", "value 1");
	form.add("key_2", "value_2");
	form.add("key_2", "一二三");
	cpv::Packet packet;
	form.buildUrlEncoded(packet);
	// packet now contains
	// "key%201=value%201&key_2=value_2&key_2=%E4%B8%80%E4%BA%8C%E4%B8%89"
}
```

If you add `dumpForm` function that takes `cpv::HttpForm&` to your model type, then you can use `cpv::serializeForm(model)` to get the url encoded form packet, for example:

``` c++
#include <CPVFramework/Serialize/FormSerializer.hpp>

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

void example {
	MyModel model;
	model.intValue = 101;
	model.sizeValue = 102;
	model.doubleValue = 0.1;
	model.stringValue = "abc";
	model.sharedStringValue = "一二三";
	model.intValues = { 1, 2, 100 };
	cpv::Packet packet = cpv::serializeForm(model);
	// packet now contains
	// "doubleValue=0.1&intValue=101&"
	// "intValues=1&intValues=2&intValues=100&"
	// "sharedStringValue=%E4%B8%80%E4%BA%8C%E4%B8%89&"
	// "sizeValue=102&stringValue=abc");
	// notice parameters are sorted by key, it's useful for assertion
}
```

### FormDeserializer

The [FormDeserializer](../include/CPVFramework/Serialize/FormDeserializer.hpp) is also based on [HttpForm](../include/CPVFramework/Http/HttpForm.hpp) provided by cpv framework, here is an example of `HttpForm`:

``` c++
#include <CPVFramework/Http/HttpForm.hpp>

void example() {
	cpv::HttpForm form;
	form.parseUrlEncoded("key_a=value_1&key_b=value_2&key_b=value_3");
	for (const auto& entry : form.getAll()) {
		std::cout << "key: " << entry.first << std::endl;
		for (const auto& value : entry.second) {
			std::cout << "value: " << value << std::endl;
		}
	}
	// outputs:
	// key: key_a
	// value: value_1
	// key: key_b
	// value: value_2
	// value: value_3

	std::cout << "a: " << form.get("key_a") << std::endl;
	std::cout << "b: " << form.get("key_b") << std::endl;
	std::cout << "c: " << form.get("key_c") << std::endl;
	// get function will use the first value if there multiples
	// outputs:
	// a: value_1
	// b: value_2
	// c:

	for (const auto& value : form.getMany("key_b")) {
		std::cout << "many b: " << value << std::endl;
	}
	// outputs:
	// many b: value_2
	// many b: value_3
}
```

If you add `loadForm` function that takes `const cpv::HttpForm&` to your model type, then you can use `cpv::deserializeForm(model, formBody)` to parse and convert the formBody to model, for example:

``` c++
#include <CPVFramework/Serialize/FormDeserializer.hpp>

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

void example() {
	cpv::SharedString formBody(
		"intValue=101&sizeValue=102&doubleValue=0.1&stringValue=abc&"
		"sharedStringValue=%E4%B8%80%E4%BA%8C%E4%B8%89&"
		"intValues=1&intValues=2&intValues=100");
	MyModel model;
	cpv::deserializeForm(model, formBody);
}
```

Notice: not like json, the `HttpForm` will ignore all errors, and the `FormDeserializer` doesn't provide an error reporting interface, you should validate your model with other method.

