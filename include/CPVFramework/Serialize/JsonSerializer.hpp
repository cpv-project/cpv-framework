#pragma once
#include <chrono>
#include <limits>
#include <memory>
#include <optional>
#include <seastar/core/shared_ptr.hh>
#include "../Allocators/StackAllocator.hpp"
#include "../Utility/ConstantStrings.hpp"
#include "../Utility/ObjectTrait.hpp"
#include "../Utility/Packet.hpp"
#include "../Utility/SharedString.hpp"

// construct JsonMemberKey, please ensure key is already encoded
#define CPV_JSONKEY(key) cpv::JsonMemberKey("\"" key "\":", ",\"" key "\":")

namespace cpv {
	/**
	 * Encode string for use in json,
	 * may return original string if not changed.
	 */
	SharedString jsonEncode(SharedString&& str);

	/**
	 * Encoded static member key of json object.
	 * Please use CPV_JSONKEY to construct it,
	 * and ensure the key pass to constructor is already encoded.
	 */
	class JsonMemberKey {
	public:
		/** Get "encoded key": */
		std::string_view getEncodedKey() const {
			return encodedKey_;
		}

		/** Get ,"encoded key": */
		std::string_view getEncodedKeyWithPreviousComma() const {
			return encodedKeyWithPreviousComma_;
		}

		/** Constructor */
		JsonMemberKey(
			std::string_view encodedKey,
			std::string_view encodedKeyWithPreviousComma) :
			encodedKey_(encodedKey),
			encodedKeyWithPreviousComma_(encodedKeyWithPreviousComma) { }

	private:
		std::string_view encodedKey_;
		std::string_view encodedKeyWithPreviousComma_;
	};

	/**
	 * The class used to build json packet.
	 * For performance reason, it won't validate the sequence of operations,
	 * you should use unit tests to ensure the generated json structure is valid.
	 */
	class JsonBuilder {
	public:
		/** Write { to json packet */
		JsonBuilder& startObject() {
			writeRaw(constants::CurlyBacketStart);
			addPreviousComma_ = false;
			return *this;
		}

		/**
		 * Write key and value to json packet.
		 * It should be used between startObject and endObject.
		 */
		template <class T>
		JsonBuilder& addMember(const SharedString& key, const T& value);

		/**
		 * Write static encoded key and value to json packet.
		 * It should be used between startObject and endObject.
		 */
		template <class T>
		JsonBuilder& addMember(const JsonMemberKey& key, const T& value);

		/** Write } to json packet */
		JsonBuilder& endObject() {
			writeRaw(constants::CurlyBacketEnd);
			addPreviousComma_ = true;
			return *this;
		}

		/** Write [ to json packet */
		JsonBuilder& startArray() {
			writeRaw(constants::SquareBacketStart);
			addPreviousComma_ = false;
			return *this;
		}

		/**
		 * Write item to json packet.
		 * It should be used between startArray and endArray.
		 */
		template <class T>
		JsonBuilder& addItem(const T& value);

		/** Write ] to json packet */
		JsonBuilder& endArray() {
			writeRaw(constants::SquareBacketEnd);
			addPreviousComma_ = true;
			return *this;
		}

		/** Write raw string */
		template <class... Args>
		void writeRaw(Args&&... args) {
			fragments_->append(std::forward<Args>(args)...);
		}

		/** Get the json packet, don't touch the json builder after invoked this */
		Packet toPacket() && {
			fragments_ = nullptr;
			return std::move(packet_);
		}

		/** Constructor with capacity of packet */
		explicit JsonBuilder(std::size_t capacity) :
			packet_(capacity),
			fragments_(&packet_.getOrConvertToMultiple()),
			addPreviousComma_(false) { }

	private:
		Packet packet_;
		Packet::MultipleFragments* fragments_;
		bool addPreviousComma_;
	};

	/**
	 * The class used to write model to json builder.
	 *
	 * The model type should contains a public function named dumpJson
	 * that takes an instance of JsonBuilder.
	 *
	 * You can specialize it for more types.
	 */
	template <class T, class = void /* for enable_if */>
	struct JsonBuilderWriter {
		/** Write model to json builder */
		static void write(const T& model, JsonBuilder& builder) {
			model.dumpJson(builder);
		}
	};

	/** Specialize for integer (except of bool) */
	template <class T>
	struct JsonBuilderWriter<T,
		std::enable_if_t<std::numeric_limits<T>::is_integer && !std::is_same_v<T, bool>>> {
		/** Write integer to json builder */
		static void write(const T& value, JsonBuilder& builder) {
			builder.writeRaw(SharedString::fromInt(value));
		}
	};

	/** Specialize for floating point */
	template <class T>
	struct JsonBuilderWriter<T,
		std::enable_if_t<std::is_floating_point_v<T>>> {
		/** Write floating point to json builder */
		static void write(const T& value, JsonBuilder& builder) {
			builder.writeRaw(SharedString::fromDouble(value));
		}
	};

	/** Specialize for bool */
	template <>
	struct JsonBuilderWriter<bool> {
		/** Write boolean to json builder */
		static void write(bool value, JsonBuilder& builder) {
			if (value) {
				builder.writeRaw(constants::True);
			} else {
				builder.writeRaw(constants::False);
			}
		}
	};

	/** Specialize for SharedString */
	template <>
	struct JsonBuilderWriter<SharedString> {
		/** Write SharedString to json builder */
		static void write(const SharedString& value, JsonBuilder& builder) {
			builder.writeRaw(constants::DoubleQuote);
			builder.writeRaw(jsonEncode(value.share()));
			builder.writeRaw(constants::DoubleQuote);
		}
	};

	/** Specialize for std::string */
	template <>
	struct JsonBuilderWriter<std::string> {
		/** Write std::string to json builder */
		static void write(const std::string& value, JsonBuilder& builder) {
			builder.writeRaw(constants::DoubleQuote);
			builder.writeRaw(jsonEncode(SharedString(value)));
			builder.writeRaw(constants::DoubleQuote);
		}
	};

	/** Specialize for collection like types */
	template <class T>
	struct JsonBuilderWriter<T, std::enable_if_t<
		ObjectTrait<T>::IsCollectionLike && !ObjectTrait<T>::IsPointerLike>> {
		/** Write collection like oject to json builder */
		static void write(const T& values, JsonBuilder& builder) {
			builder.startArray();
			ObjectTrait<T>::apply(values, [&builder] (const auto& value) {
				builder.addItem(value);
			});
			builder.endArray();
		}
	};

	/** Specialize for pointer like types */
	template <class T>
	struct JsonBuilderWriter<T, std::enable_if_t<ObjectTrait<T>::IsPointerLike>> {
		/** Write pointer like object to json builder */
		static void write(const T& value, JsonBuilder& builder) {
			using UnderlyingType = typename ObjectTrait<T>::UnderlyingType;
			const UnderlyingType* ptr = ObjectTrait<T>::get(value);
			if (ptr != nullptr) {
				JsonBuilderWriter<UnderlyingType>::write(*ptr, builder);
			} else {
				builder.writeRaw(constants::Null);
			}
		}
	};

	/** Specialize for chrono durations */
	template <class Rep, class Period>
	struct JsonBuilderWriter<std::chrono::duration<Rep, Period>> {
		/** Write chrono durations to json builder */
		static void write(
			const std::chrono::duration<Rep, Period>& value, JsonBuilder& builder) {
			builder.writeRaw(SharedString::fromInt(value.count()));
		}
	};

	/** Write key and value to json packet */
	template <class T>
	JsonBuilder& JsonBuilder::addMember(const SharedString& key, const T& value) {
		if (addPreviousComma_) {
			writeRaw(constants::Comma);
		} else {
			addPreviousComma_ = true;
		}
		JsonBuilderWriter<SharedString>::write(key, *this);
		writeRaw(constants::Colon);
		JsonBuilderWriter<T>::write(value, *this);
		return *this;
	}

	/** Write static encoded key and value to json packet */
	template <class T>
	JsonBuilder& JsonBuilder::addMember(const JsonMemberKey& key, const T& value) {
		if (addPreviousComma_) {
			writeRaw(SharedString::fromStatic(key.getEncodedKeyWithPreviousComma()));
		} else {
			writeRaw(SharedString::fromStatic(key.getEncodedKey()));
			addPreviousComma_ = true;
		}
		JsonBuilderWriter<T>::write(value, *this);
		return *this;
	}

	/** Write item to json packet */
	template <class T>
	JsonBuilder& JsonBuilder::addItem(const T& value) {
		if (addPreviousComma_) {
			writeRaw(constants::Comma);
		} else {
			addPreviousComma_ = true;
		}
		JsonBuilderWriter<T>::write(value, *this);
		return *this;
	}

	/**
	 * The class used to serialize model to json packet.
	 * The model should be writable to JsonBuilder from JsonBuilderWriter.
	 * If you want to build a json without model, you can use JsonBuilder directly.
	 */
	template <class T, class = void /* for enable_if */>
	class JsonSerializer {
	public:
		/** The default packet capacity */
		static const constexpr std::size_t PacketCapacity = 128;

		/** Serialize model to json packet */
		static Packet serialize(const T& model) {
			JsonBuilder builder(PacketCapacity);
			JsonBuilderWriter<T>::write(model, builder);
			return std::move(builder).toPacket();
		}
	};

	/** Convenient static function for JsonSerializer */
	template <class T>
	static inline Packet serializeJson(const T& model) {
		return JsonSerializer<T>::serialize(model);
	}
}

