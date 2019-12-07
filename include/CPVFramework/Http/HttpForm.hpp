#pragma once
#include "../Allocators/StackAllocator.hpp"
#include "../Utility/Packet.hpp"

namespace cpv {
	/**
	 * Class used to parse and build url encoded form
	 *
	 * Example (url encoded):
	 * a=1&b=2&b=3
	 * ```
	 * formParameters: { "a": [ "1" ], b: [ "2", "3" ] }
	 * ```
	 *
	 * Notice:
	 * Duplicated form parameter is supported, you can use getMany() to get all values,
	 * or use get() to get the first value.
	 * For performance reason, form parser will ignore all errors.
	 *
	 * TODO: add parseMultipart and buildMultipart, may require layout change for files.
	 */
	class HttpForm {
	public:
		using ValuesType = StackAllocatedVector<SharedString, 1>;
		using FormParametersType = StackAllocatedMap<SharedString, ValuesType, 8>;

		/** Get all values */
		const FormParametersType& getAll() const& { return formParameters_; }
		FormParametersType& getAll() & { return formParameters_; }

		/**
		 * Get the first value associated with given key.
		 * return empty if key not exists.
		 */
		SharedString get(const SharedString& key) const {
			auto it = formParameters_.find(key);
			if (it != formParameters_.end() && !it->second.empty()) {
				return it->second.front().share();
			}
			return SharedString();
		}

		/**
		 * Get values associated with given key.
		 * return a static empty vector if key not exists.
		 */
		const ValuesType& getMany(const SharedString& key) const& {
			auto it = formParameters_.find(key);
			return (it != formParameters_.end()) ? it->second : Empty;
		}

		/**
		 * Add value associated with given key.
		 * Call it multiple times can associate multiple values with the same key.
		 */
		void add(SharedString&& key, SharedString&& value) {
			formParameters_[std::move(key)].emplace_back(std::move(value));
		}

		/** Remove values associated with given key */
		void remove(const SharedString& key) {
			formParameters_.erase(key);
		}

		/** Remove all values */
		void clear() {
			formParameters_.clear();
		}

		/** Parse url encoded form body */
		void parseUrlEncoded(const SharedString& body);

		/** Apend url encoded form body to packet */
		void buildUrlEncoded(Packet& packet);

		/** Constructor */
		HttpForm();

		/** Construct with url encoded form body */
		explicit HttpForm(const SharedString& body) : HttpForm() {
			parseUrlEncoded(body);
		}

	private:
		static const ValuesType Empty;

	private:
		FormParametersType formParameters_;
	};
}

