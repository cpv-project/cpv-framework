#pragma once
#include <cassert>
#include <string_view>
#include <seastar/core/temporary_buffer.hh>
#include "./ConstantStrings.hpp"

namespace cpv {
	/**
	 * Reference counted slice of string.
	 *
	 * It's an extension of seastar::temporary_buffer with better string view support.
	 *
	 * Notice:
	 * It can point to a static const buffer (const_cast is used),
	 * if you need to modify the buffer, please ensure the buffer is not static const.
	 *
	 * Conversion:
	 * convert from std::string_view: explicit (copy)
	 * convert to std::string_view: implicit
	 * convert from seastar::temporary_buffer: implicit
	 */
	template <class CharType>
	class BasicSharedString : private seastar::temporary_buffer<CharType> {
	private:
		using Base = seastar::temporary_buffer<CharType>;
		using View = std::basic_string_view<CharType>;

	public:
		using Base::temporary_buffer;
		using Base::operator[];
		using Base::operator bool;
		using Base::size;
		using Base::begin;
		using Base::end;
		using Base::empty;
		using Base::trim_front;
		using Base::trim;
		using Base::release;

		/** Get a pointer to the beginning of the buffer */
		CharType* data() & { return Base::get_write(); }
		const CharType* data() const& { return Base::get(); }

		/** Convert to string view */
		View view() const& { return { data(), size() }; }
		operator View() const& { return { data(), size() }; }

		/** Convert to temporary buffer */
		Base buffer() & { return Base::share(); }
		Base buffer() && { Base buf(data(), size(), release()); trim(0); return buf; }

		/** Create an instance that shared the same storage */
		BasicSharedString share() const {
			// use const_cast to make this function be const, only refcount will change
			return BasicSharedString(
				const_cast<BasicSharedString*>(this)->Base::share());
		}

		/** Create an instance that shared the same storage with given range */
		BasicSharedString share(std::size_t pos, std::size_t len) const {
			assert(len == 0 || (pos <= size() && size() - pos >= len));
			// use const_cast to make this function be const, only refcount will change
			return BasicSharedString(
				const_cast<BasicSharedString*>(this)->Base::share(pos, len));
		}

		/** Create an instance that shared the same storage with given range */
		BasicSharedString share(std::size_t pos) const {
			return share(pos, size() - pos);
		}

		/** Create an instance that shared the same storage with given sub view */
		BasicSharedString share(std::string_view view) const {
			return share(view.data() - begin(), view.size());
		}

		/** Trim this string by it's sub view */
		void trim(std::string_view view) {
			assert(view.size() == 0 || (view.data() >= begin() && view.data() <= end()));
			assert(view.size() == 0 ||
				(static_cast<std::size_t>(end() - view.data()) >= view.size()));
			trim_front(view.data() - begin());
			trim(view.size());
		}

		/** Construct with data copied from given string view */
		explicit BasicSharedString(View view) :
			BasicSharedString(view.data(), view.size()) { }

		/** Construct with static string (use empty deleter) */
		// cppcheck-suppress noExplicitConstructor
		template <std::size_t Size>
		BasicSharedString(const CharType(&str)[Size]) :
			BasicSharedString(const_cast<CharType*>(str), Size - 1, seastar::deleter()) {
			static_assert(Size >= 1, "static string should contains tailing zero");
		}

		/** Construct with seastar::temporary_buffer */
		// cppcheck-suppress noExplicitConstructor
		BasicSharedString(Base&& buffer) :
			Base(std::move(buffer)) { }

		/** Compare with other shared string (for used as key in map and set) */
		bool operator ==(const BasicSharedString& other) const { return view() == other.view(); }
		bool operator !=(const BasicSharedString& other) const { return view() != other.view(); }
		bool operator <(const BasicSharedString& other) const { return view() < other.view(); }
		bool operator <=(const BasicSharedString& other) const { return view() <= other.view(); }
		bool operator >(const BasicSharedString& other) const { return view() > other.view(); }
		bool operator >=(const BasicSharedString& other) const { return view() >= other.view(); }

		/** Compare with other string view */
		bool operator ==(std::string_view other) const { return view() == other; }
		bool operator !=(std::string_view other) const { return view() != other; }
		bool operator <(std::string_view other) const { return view() < other; }
		bool operator <=(std::string_view other) const { return view() <= other; }
		bool operator >(std::string_view other) const { return view() > other; }
		bool operator >=(std::string_view other) const { return view() >= other; }

		/** Compare with other static const string (we need c++20 spaceship operator...) */
		template <std::size_t Size>
		bool operator ==(const CharType(&str)[Size]) const { return *this == BasicSharedString(str); }
		template <std::size_t Size>
		bool operator !=(const CharType(&str)[Size]) const { return *this != BasicSharedString(str); }
		template <std::size_t Size>
		bool operator <(const CharType(&str)[Size]) const { return *this < BasicSharedString(str); }
		template <std::size_t Size>
		bool operator <=(const CharType(&str)[Size]) const { return *this <= BasicSharedString(str); }
		template <std::size_t Size>
		bool operator >(const CharType(&str)[Size]) const { return *this > BasicSharedString(str); }
		template <std::size_t Size>
		bool operator >=(const CharType(&str)[Size]) const { return *this >= BasicSharedString(str); }

	private:
		static BasicSharedString fromIntImpl(std::intmax_t value);
		static BasicSharedString fromIntImpl(std::uintmax_t value);
		static BasicSharedString fromDoubleImpl(double value);
		static BasicSharedString fromDoubleImpl(long double value);

	public:
		/** Construct with static string */
		static BasicSharedString fromStatic(View view) {
			return BasicSharedString(
				const_cast<CharType*>(view.data()), view.size(), seastar::deleter());
		}

		/** Construct with string representation of integer */
		template <class T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
		static BasicSharedString fromInt(T value) {
			if constexpr (std::numeric_limits<T>::is_signed) {
				if (value >= 0) {
					if (static_cast<std::uintmax_t>(value) < constants::Integers.size()) {
						return BasicSharedString::fromStatic(constants::Integers[value]);
					} else {
						return fromIntImpl(static_cast<std::uintmax_t>(value));
					}
				} else {
					return fromIntImpl(static_cast<std::intmax_t>(value));
				}
			} else {
				if (value < constants::Integers.size()) {
					return BasicSharedString::fromStatic(constants::Integers[value]);
				} else {
					return fromIntImpl(static_cast<std::uintmax_t>(value));
				}
			}
		}

		/** Construct with string representation of floating point */
		template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
		static BasicSharedString fromDouble(T value) {
			std::size_t intValue = value;
			if (value == static_cast<T>(intValue) && intValue < constants::Integers.size()) {
				return BasicSharedString(constants::Integers[intValue]);
			}
			if constexpr (sizeof(T) <= sizeof(double)) {
				return fromDoubleImpl(static_cast<double>(value));
			} else {
				return fromDoubleImpl(static_cast<long double>(value));
			}
		}
	};

	/** Print string */
	template <class CharType>
	std::ostream& operator<<(
		std::ostream& stream, const BasicSharedString<CharType>& str) {
		return stream << str.view();
	}

	// Type aliases
	using SharedString = BasicSharedString<char>;
}

namespace std {
	/** Provide std::hash for BasicSharedString<CharType> */
	template <class CharType>
	struct hash<cpv::BasicSharedString<CharType>> {
		std::size_t operator()(const cpv::BasicSharedString<CharType>& str) const {
			return std::hash<std::basic_string_view<CharType>>()(str.view());
		}
	};
}

