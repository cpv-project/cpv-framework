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
			if (size_ == 0) {
				// newSize may be pre calculated (like append integer)
				buffer_ = BasicSharedString<CharType>(newSize);
			} else {
				// use std::max to handle size * 2 overflow
				BasicSharedString<CharType> newBuffer(std::max(
					std::max(newSize, buffer_.size() * 2), +MinCapacityForGrow));
				std::memcpy(newBuffer.data(), buffer_.data(), size_ * sizeof(CharType));
				buffer_ = std::move(newBuffer);
			}
		}

		/** Grow the buffer for newly append size if necessary and update the view */
		CharType* grow(std::size_t appendSize) {
			std::size_t oldSize = size_;
			if (CPV_UNLIKELY(std::numeric_limits<std::size_t>::max() - oldSize < appendSize)) {
				throw OverflowException(CPV_CODEINFO, "size overflowed");
			}
			std::size_t newSize = oldSize + appendSize;
			reserve(newSize);
			size_ = newSize;
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
			size_ = size;
		}

		/** Reset the view of result string (capacity will remain) */
		void clear() {
			size_ = 0;
		}

		/** Get the view of result string */
		std::string_view view() const {
			return { buffer_.data(), size_ };
		}

		/** Get the result string, the builder will become empty after invoked */
		BasicSharedString<CharType> build() {
			buffer_.trim(size_);
			size_ = 0;
			return std::move(buffer_);
		}

		// Convenient functions
		std::size_t size() const { return size_; }
		std::size_t capacity() const { return buffer_.size(); }
		bool empty() const { return size_ == 0; }
		CharType* data() { return buffer_.data(); }
		const CharType* data() const { return buffer_.data(); }
		CharType* begin() { return buffer_.data(); }
		const CharType* begin() const { return buffer_.data(); }
		CharType* end() { return buffer_.data() + size_; }
		const CharType* end() const { return buffer_.data() + size_; }

		/** Constructor */
		BasicSharedStringBuilder() :
			buffer_(),
			size_(0) { }

		/** Construct with initial capacity */
		explicit BasicSharedStringBuilder(std::size_t capacity) :
			buffer_(capacity),
			size_(0) { }

		/** Construct with initial buffer */
		explicit BasicSharedStringBuilder(BasicSharedString<CharType>&& str) :
			buffer_(std::move(str)),
			size_(buffer_.size()) { }

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
		std::size_t size_;
	};

	// Type aliases
	using SharedStringBuilder = BasicSharedStringBuilder<char>;
}

