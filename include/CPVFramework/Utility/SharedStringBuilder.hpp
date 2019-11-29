#pragma once
#include <cstring>
#include <cassert>
#include <algorithm>
#include "../Exceptions/OverflowException.hpp"
#include "./SharedString.hpp"

namespace cpv {
	/**
	 * Class used to build shared string
	 * It can grow the internal buffer automatically when append new contents.
	 */
	template <class CharType>
	class BasicSharedStringBuilder {
	public:
		/** Minimal capacity for new buffer allocated from grow (except for grow from empty) */
		static const constexpr std::size_t MinCapacityForGrow = 512;

		/** Grow the buffer for given total size if necessary */
		void reserve(std::size_t newSize) {
			if (newSize <= buffer_.size()) {
				return;
			}
			// allocate new buffer and copy original content
			if (view_.empty()) {
				// newSize may be pre calculated (like append integer)
				buffer_ = BasicSharedString<CharType>(newSize);
			} else {
				// use std::max to handle size * 2 overflow
				BasicSharedString<CharType> newBuffer(std::max(
					std::max(newSize, buffer_.size() * 2), +MinCapacityForGrow));
				std::memcpy(newBuffer.data(), view_.data(), view_.size() * sizeof(CharType));
				buffer_ = std::move(newBuffer);
			}
		}

		/** Grow the buffer for newly append size if necessary and update the view */
		CharType* grow(std::size_t appendSize) {
			std::size_t oldSize = view_.size();
			if (CPV_UNLIKELY(std::numeric_limits<std::size_t>::max() - oldSize < appendSize)) {
				throw OverflowException(CPV_CODEINFO, "size overflowed");
			}
			std::size_t newSize = oldSize + appendSize;
			reserve(newSize);
			view_ = { buffer_.data(), newSize };
			return buffer_.data() + oldSize;
		}

		/** Append count copies of character c to the end */
		BasicSharedStringBuilder& append(std::size_t count, CharType c) {
			std::fill_n(grow(count), count, c);
			return *this;
		}

		/** Append string view to end */
		BasicSharedStringBuilder& append(std::basic_string_view<CharType> view) {
			if (CPV_LIKELY(!view.empty())) {
				std::memcpy(grow(view.size()), view.data(), view.size() * sizeof(CharType));
			}
			return *this;
		}

		/** Append string representation of integer to end */
		template <class T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
		BasicSharedStringBuilder& append(T value) {
			if constexpr (std::numeric_limits<T>::is_signed) {
				return appendImpl(static_cast<std::intmax_t>(value));
			} else {
				return appendImpl(static_cast<std::uintmax_t>(value));
			}
		}

		/** Append string representation of floating point to end */
		template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
		BasicSharedStringBuilder& append(T value) {
			if constexpr (sizeof(T) <= sizeof(double)) {
				return appendImpl(static_cast<double>(value));
			} else {
				return appendImpl(static_cast<long double>(value));
			}
		}

		/** Resize the view of result string, the size must not greater than the capacity */
		void resize(std::size_t size) {
			assert(size <= buffer_.size());
			view_ = { buffer_.data(), size };
		}

		/** Reset the view of result string (capacity will remain) */
		void clear() {
			view_ = {};
		}

		/** Get the view of result string */
		std::string_view view() const {
			return view_;
		}

		/** Get the result string, the builder will become empty after invoked */
		BasicSharedString<CharType> build() {
			buffer_.trim(view_.size());
			view_ = {};
			return std::move(buffer_);
		}

		// Convenient functions
		std::size_t size() const { return view_.size(); }
		std::size_t capacity() const { return buffer_.size(); }
		bool empty() const { return view_.empty(); }
		CharType* data() { return buffer_.data(); }
		const CharType* data() const { return buffer_.data(); }
		CharType* begin() { return buffer_.data(); }
		const CharType* begin() const { return buffer_.data(); }
		CharType* end() { return buffer_.data() + view_.size(); }
		const CharType* end() const { return buffer_.data() + view_.size(); }

		/** Constructor */
		BasicSharedStringBuilder() :
			buffer_(),
			view_() { }

		/** Construct with initial capacity */
		explicit BasicSharedStringBuilder(std::size_t capacity) :
			buffer_(capacity),
			view_() { }

		/** Construct with initial buffer */
		explicit BasicSharedStringBuilder(BasicSharedString<CharType>&& str) :
			buffer_(std::move(str)),
			view_(buffer_.view()) { }

	private:
		/** Append string representation of signed integer to end */
		BasicSharedStringBuilder& appendImpl(std::intmax_t value);

		/** Append string representation of unsigned integer to end */
		BasicSharedStringBuilder& appendImpl(std::uintmax_t value);

		/** Append string representation of double to end */
		BasicSharedStringBuilder& appendImpl(double value);

		/** Append string representation of long double to end */
		BasicSharedStringBuilder& appendImpl(long double value);

	private:
		BasicSharedString<CharType> buffer_;
		std::string_view view_;
	};

	// Type aliases
	using SharedStringBuilder = BasicSharedStringBuilder<char>;
}

