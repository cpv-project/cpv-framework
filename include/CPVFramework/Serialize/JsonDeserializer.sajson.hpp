/*
 * Copyright (c) 2012-2017 Chad Austin
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// sajson 2dcfd350586375f9910f74821d4f07d67ae455ba 2018-09-21
// original sajson is header only but I split the parser part to src for reducing binary size

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <algorithm>
#include <cstdio>
#include <limits>

#ifndef SAJSON_NO_STD_STRING
#include <string> // for convenient access to error messages and string values.
#include <string_view>
#endif

#if defined(__GNUC__) || defined(__clang__)
#define SAJSON_LIKELY(x) __builtin_expect(!!(x), 1)
#define SAJSON_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define SAJSON_ALWAYS_INLINE __attribute__((always_inline))
#define SAJSON_UNREACHABLE() __builtin_unreachable()
#define SAJSON_snprintf snprintf
#elif defined(_MSC_VER)
#define SAJSON_LIKELY(x) x
#define SAJSON_UNLIKELY(x) x
#define SAJSON_ALWAYS_INLINE __forceinline
#define SAJSON_UNREACHABLE() __assume(0)
#if (_MSC_VER <= 1800)
#define SAJSON_snprintf _snprintf
#else
#define SAJSON_snprintf snprintf
#endif
#else
#define SAJSON_LIKELY(x) x
#define SAJSON_UNLIKELY(x) x
#define SAJSON_ALWAYS_INLINE inline
#define SAJSON_UNREACHABLE() assert(!"unreachable")
#define SAJSON_snprintf snprintf
#endif

/**
 * sajson Public API
 */
namespace sajson {

	/// Tag indicating a JSON value's type.
	enum type: uint8_t {
		TYPE_INTEGER = 0,
		TYPE_DOUBLE = 1,
		TYPE_NULL = 2,
		TYPE_FALSE = 3,
		TYPE_TRUE = 4,
		TYPE_STRING = 5,
		TYPE_ARRAY = 6,
		TYPE_OBJECT = 7,
		TYPE_NOKEY = 255
	};
	
	namespace internal {
		static const size_t TYPE_BITS = 3;
		static const size_t TYPE_MASK = (1 << TYPE_BITS) - 1;
		static const size_t VALUE_MASK = size_t(-1) >> TYPE_BITS;
	
		static const size_t ROOT_MARKER = VALUE_MASK;
	
		inline type get_element_type(size_t s) {
			return static_cast<type>(s & TYPE_MASK);
		}
	
		inline size_t get_element_value(size_t s) {
			return s >> TYPE_BITS;
		}
	
		inline size_t make_element(type t, size_t value) {
			//assert((value & ~VALUE_MASK) == 0);
			//value &= VALUE_MASK;
			return static_cast<size_t>(t) | (value << TYPE_BITS);
		}
			
		// This template utilizes the One Definition Rule to create global arrays in a header.
		// This trick courtesy of Rich Geldreich's Purple JSON parser.
		template<typename unused=void>
		struct globals_struct {
			static const unsigned char parse_flags[256];
		};
		typedef globals_struct<> globals;

		// bit 0 (1) - set if: plain ASCII string character
		// bit 1 (2) - set if: whitespace
		// bit 4 (0x10) - set if: 0-9 e E .
		template<typename unused>
		const uint8_t globals_struct<unused>::parse_flags[256] = {
		 // 0	1	2	3	4	5	6	7	  8	9	A	B	C	D	E	F
			0,   0,   0,   0,   0,   0,   0,   0,	 0,   2,   2,   0,   0,   2,   0,   0, // 0
			0,   0,   0,   0,   0,   0,   0,   0,	 0,   0,   0,   0,   0,   0,   0,   0, // 1
			3,   1,   0,   1,   1,   1,   1,   1,	 1,   1,   1,   1,   1,   1,   0x11,1, // 2
			0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,  0x11,0x11,1,   1,   1,   1,   1,   1, // 3
			1,   1,   1,   1,   1,   0x11,1,   1,	 1,   1,   1,   1,   1,   1,   1,   1, // 4
			1,   1,   1,   1,   1,   1,   1,   1,	 1,   1,   1,   1,   0,   1,   1,   1, // 5
			1,   1,   1,   1,   1,   0x11,1,   1,	 1,   1,   1,   1,   1,   1,   1,   1, // 6
			1,   1,   1,   1,   1,   1,   1,   1,	 1,   1,   1,   1,   1,   1,   1,   1, // 7

		 // 128-255
			0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0
		};

		inline bool is_plain_string_character(char c) {
			//return c >= 0x20 && c <= 0x7f && c != 0x22 && c != 0x5c;
			return (globals::parse_flags[static_cast<unsigned char>(c)] & 1) != 0;
		}

		inline bool is_whitespace(char c) {
			//return c == '\r' || c == '\n' || c == '\t' || c == ' ';
			return (globals::parse_flags[static_cast<unsigned char>(c)] & 2) != 0;
		}

		class allocated_buffer {
		public:
			allocated_buffer()
				: memory(0)
			{}

			explicit allocated_buffer(size_t length) {
				// throws std::bad_alloc upon allocation failure
				void* buffer = operator new(sizeof(size_t) + length);
				memory = static_cast<layout*>(buffer);
				memory->refcount = 1;
			}

			allocated_buffer(const allocated_buffer& that)
				: memory(that.memory)
			{
				incref();
			}

			allocated_buffer(allocated_buffer&& that)
				: memory(that.memory)
			{
				that.memory = 0;
			}

			~allocated_buffer() {
				decref();
			}

			allocated_buffer& operator=(const allocated_buffer& that) {
				if (this != &that) {
					decref();
					memory = that.memory;
					incref();
				}
				return *this;
			}

			allocated_buffer& operator=(allocated_buffer&& that) {
				if (this != &that) {
					decref();
					memory = that.memory;
					that.memory = 0;
				}
				return *this;
			}

			char* get_data() const {
				return memory ? memory->data : 0;
			}

		private:
			void incref() const {
				if (memory) {
					++(memory->refcount);
				}
			}

			void decref() const {
				if (memory && --(memory->refcount) == 0) {
					operator delete(memory);
				}
			}

			struct layout {
				size_t refcount;
				char data[];
			};

			layout* memory;
		};
	}

	/// A simple type encoding a pointer to some memory and a length (in bytes).
	/// Does not maintain any memory.
	class string {
	public:
		string(const char* text_, size_t length)
			: text(text_)
			, _length(length)
		{}

		const char* data() const {
			return text;
		}

		size_t length() const {
			return _length;
		}

#ifndef SAJSON_NO_STD_STRING
		std::string as_string() const {
			return std::string(text, text + _length);
		}
#endif

	private:
		const char* const text;
		const size_t _length;

		string(); /*=delete*/
	};

	/// A convenient way to parse JSON from a string literal.  The string ends
	/// at its first NUL character.
	class literal : public string {
	public:
		template <size_t sz>
		explicit literal(const char (&text_)[sz])
			: string(text_, sz - 1)
		{
			static_assert(sz > 0, "!");
		}
	};

	/// A pointer to a mutable buffer, its size in bytes, and strong ownership of any
	/// copied memory.
	class mutable_string_view {
	public:
		/// Creates an empty, zero-sized view.
		mutable_string_view()
			: length_(0)
			, data(0)
			, buffer()
		{}

		/// Given a length in bytes and a pointer, constructs a view
		/// that does not allocate a copy of the data or maintain its life.
		/// The given pointer must stay valid for the duration of the parse and the
		/// resulting \ref document's life.
		mutable_string_view(size_t length, char* data_)
			: length_(length)
			, data(data_)
			, buffer()
		{}

		/// Allocates a copy of the given \ref literal string and exposes a
		/// mutable view into it.  Throws std::bad_alloc if allocation fails.
		explicit mutable_string_view(const literal& s)
			: length_(s.length())
			, buffer(length_)
		{
			data = buffer.get_data();
			memcpy(data, s.data(), length_);
		}

		/// Allocates a copy of the given \ref string and exposes a mutable view
		/// into it.  Throws std::bad_alloc if allocation fails.
		explicit mutable_string_view(const string& s)
			: length_(s.length())
			, buffer(length_)
		{
			data = buffer.get_data();
			memcpy(data, s.data(), length_);
		}

		/// Copies a mutable_string_view.  If any backing memory has been
		/// allocated, its refcount is incremented - both views can safely
		/// use the memory.
		mutable_string_view(const mutable_string_view& that)
			: length_(that.length_)
			, data(that.data)
			, buffer(that.buffer)
		{}

		/// Move constructor - neuters the old mutable_string_view.
		mutable_string_view(mutable_string_view&& that)
			: length_(that.length_)
			, data(that.data)
			, buffer(std::move(that.buffer))
		{
			that.length_ = 0;
			that.data = 0;
		}

		mutable_string_view& operator=(mutable_string_view&& that) {
			if (this != &that) {
				length_ = that.length_;
				data = that.data;
				buffer = std::move(that.buffer);
				that.length_ = 0;
				that.data = 0;
			}
			return *this;
		}

		mutable_string_view& operator=(const mutable_string_view& that) {
			if (this != &that) {
				length_ = that.length_;
				data = that.data;
				buffer = that.buffer;
			}
			return *this;
		}

		size_t length() const {
			return length_;
		}

		char* get_data() const {
			return data;
		}

	private:
		size_t length_;
		char* data;
		internal::allocated_buffer buffer; // may not be allocated
	};

	namespace internal {
		struct object_key_record {
			size_t key_start;
			size_t key_end;
			size_t value;
		};

		struct object_key_comparator {
			explicit object_key_comparator(const char* object_data)
				: data(object_data)
			{}

			bool operator()(const object_key_record& lhs, const string& rhs) const {
				const size_t lhs_length = lhs.key_end - lhs.key_start;
				const size_t rhs_length = rhs.length();
				if (lhs_length < rhs_length) {
					return true;
				} else if (lhs_length > rhs_length) {
					return false;
				}
				return memcmp(data + lhs.key_start, rhs.data(), lhs_length) < 0;
			}

			bool operator()(const string& lhs, const object_key_record& rhs) const {
				return !(*this)(rhs, lhs);
			}

			bool operator()(
				const object_key_record& lhs,
				const object_key_record& rhs
			) {
				const size_t lhs_length = lhs.key_end - lhs.key_start;
				const size_t rhs_length = rhs.key_end - rhs.key_start;
				if (lhs_length < rhs_length) {
					return true;
				} else if (lhs_length > rhs_length) {
					return false;
				}
				return memcmp(
					data + lhs.key_start,
					data + rhs.key_start,
					lhs_length
				) < 0;
			}

			const char* data;
		};
	}

	namespace integer_storage {
		enum {
			word_length = 1
		};

		inline int load(const size_t* location) {
			int value;
			memcpy(&value, location, sizeof(value));
			return value;
		}

		inline void store(size_t* location, int value) {
			// NOTE: Most modern compilers optimize away this constant-size
			// memcpy into a single instruction. If any don't, and treat
			// punning through a union as legal, they can be special-cased.
			static_assert(
				sizeof(value) <= sizeof(*location),
				"size_t must not be smaller than int");
			memcpy(location, &value, sizeof(value));
		}
	}

	namespace double_storage {
		enum {
			word_length = sizeof(double) / sizeof(size_t)
		};

		inline double load(const size_t* location) {
			double value;
			memcpy(&value, location, sizeof(double));
			return value;
		}

		inline void store(size_t* location, double value) {
			// NOTE: Most modern compilers optimize away this constant-size
			// memcpy into a single instruction. If any don't, and treat
			// punning through a union as legal, they can be special-cased.
			memcpy(location, &value, sizeof(double));
		}
	}

	/// Represents a JSON value.  First, call get_type() to check its type,
	/// which determines which methods are available.
	///
	/// Note that \ref value does not maintain any backing memory, only the
	/// corresponding \ref document does.  It is illegal to access a \ref value
	/// after its \ref document has been destroyed.
	class value {
	public:
		/// Returns the JSON value's \ref type.
		type get_type() const {
			return value_type;
		}

		/// Returns the length of the object or array.
		/// Only legal if get_type() is TYPE_ARRAY or TYPE_OBJECT.
		size_t get_length() const {
			assert_type_2(TYPE_ARRAY, TYPE_OBJECT);
			return payload[0];
		}

		/// Returns the nth element of an array.  Calling with an out-of-bound
		/// index is undefined behavior.
		/// Only legal if get_type() is TYPE_ARRAY.
		value get_array_element(size_t index) const {
			using namespace internal;
			assert_type(TYPE_ARRAY);
			size_t element = payload[1 + index];
			return value(get_element_type(element), payload + get_element_value(element), text);
		}

		/// Returns the nth key of an object.  Calling with an out-of-bound
		/// index is undefined behavior.
		/// Only legal if get_type() is TYPE_OBJECT.
		string get_object_key(size_t index) const {
			assert_type(TYPE_OBJECT);
			const size_t* s = payload + 1 + index * 3;
			return string(text + s[0], s[1] - s[0]);
		}

		/// Returns the nth value of an object.  Calling with an out-of-bound
		/// index is undefined behavior.  Only legal if get_type() is TYPE_OBJECT.
		value get_object_value(size_t index) const {
			using namespace internal;
			assert_type(TYPE_OBJECT);
			size_t element = payload[3 + index * 3];
			return value(get_element_type(element), payload + get_element_value(element), text);
		}

		/// Given a string key, returns the value with that key or a value with TYPE_NOKEY
		/// if the key is not found.  Running time is O(lg N).
		/// Only legal if get_type() is TYPE_OBJECT.
		value get_value_of_key(const string& key) const {
			assert_type(TYPE_OBJECT);
			size_t i = find_object_key(key);
			if (i < get_length()) {
				return get_object_value(i);
			} else {
				return value(TYPE_NOKEY, 0, 0);
			}
		}

		/// Given a string key, returns the index of the associated value if
		/// one exists.  Returns get_length() if there is no such key.
		/// Note: sajson sorts object keys, so the running time is O(lg N).
		/// Only legal if get_type() is TYPE_OBJECT
		size_t find_object_key(const string& key) const {
			using namespace internal;
			assert_type(TYPE_OBJECT);
			const object_key_record* start = reinterpret_cast<const object_key_record*>(payload + 1);
			const object_key_record* end = start + get_length();
#ifdef SAJSON_UNSORTED_OBJECT_KEYS
			for (const object_key_record* i = start; i != end; ++i)
#else
			const object_key_record* i = std::lower_bound(start, end, key, object_key_comparator(text));
#endif
			if (i != end
					&& (i->key_end - i->key_start) == key.length()
					&& memcmp(key.data(), text + i->key_start, key.length()) == 0) {
				return i - start;
			}
			return get_length();
		}

		/// If a numeric value was parsed as a 32-bit integer, returns it.
		/// Only legal if get_type() is TYPE_INTEGER. 
		int get_integer_value() const {
			assert_type(TYPE_INTEGER);
			return integer_storage::load(payload);
		}

		/// If a numeric value was parsed as a double, returns it.
		/// Only legal if get_type() is TYPE_DOUBLE.
		double get_double_value() const {
			assert_type(TYPE_DOUBLE);
			return double_storage::load(payload);
		}

		/// Returns a numeric value as a double-precision float.
		/// Only legal if get_type() is TYPE_INTEGER or TYPE_DOUBLE.
		double get_number_value() const {
			assert_type_2(TYPE_INTEGER, TYPE_DOUBLE);
			if (get_type() == TYPE_INTEGER) {
				return get_integer_value();
			} else {
				return get_double_value();
			}
		}

		/// Returns true and writes to the output argument if the numeric value
		/// fits in a 53-bit integer.  This is useful for timestamps and other
		/// situations where integral values with greater than 32-bit precision
		/// are used, as 64-bit values are not understood by all JSON
		/// implementations or languages.
		/// Returns false if the value is not an integer or not in range.
		/// Only legal if get_type() is TYPE_INTEGER or TYPE_DOUBLE.
		bool get_int53_value(int64_t* out) const {
			// Make sure the output variable is always defined to avoid any
			// possible situation like
			// https://gist.github.com/chadaustin/2c249cb850619ddec05b23ca42cf7a18
			*out = 0;

			assert_type_2(TYPE_INTEGER, TYPE_DOUBLE);
			if (get_type() == TYPE_INTEGER) {
				*out = get_integer_value();
				return true;
			} else if (get_type() == TYPE_DOUBLE) {
				double v = get_double_value();
				if (v < -(1LL << 53) || v > (1LL << 53)) {
					return false;
				}
				int64_t as_int = static_cast<int64_t>(v);
				if (as_int != v) {
					return false;
				}
				*out = as_int;
				return true;
			} else {
				return false;
			}
		}

		/// Returns the length of the string.
		/// Only legal if get_type() is TYPE_STRING.
		size_t get_string_length() const {
			assert_type(TYPE_STRING);
			return payload[1] - payload[0];
		}

		/// Returns a pointer to the beginning of a string value's data.
		/// WARNING: Calling this function and using the return value as a
		/// C-style string (that is, without also using get_string_length())
		/// will cause the string to appear truncated if the string has
		/// embedded NULs.
		/// Only legal if get_type() is TYPE_STRING.
		const char* as_cstring() const {
			assert_type(TYPE_STRING);
			return text + payload[0];
		}

#ifndef SAJSON_NO_STD_STRING
		/// Returns a string's value as a std::string.
		/// Only legal if get_type() is TYPE_STRING.
		std::string as_string() const {
			assert_type(TYPE_STRING);
			return std::string(text + payload[0], text + payload[1]);
		}
#endif

		/// \cond INTERNAL
		const size_t* _internal_get_payload() const {
			return payload;
		}
		/// \endcond

	private:
		explicit value(type value_type_, const size_t* payload_, const char* text_)
			: value_type(value_type_)
			, payload(payload_)
			, text(text_)
		{}

		void assert_type(type expected) const {
			assert(expected == get_type());
		}

		void assert_type_2(type e1, type e2) const {
			assert(e1 == get_type() || e2 == get_type());
		}

		const type value_type;
		const size_t* const payload;
		const char* const text;

		friend class document;
	};

	/// Error code indicating why parse failed.
	enum error {
		ERROR_NO_ERROR,
		ERROR_OUT_OF_MEMORY,
		ERROR_UNEXPECTED_END,
		ERROR_MISSING_ROOT_ELEMENT,
		ERROR_BAD_ROOT,
		ERROR_EXPECTED_COMMA,
		ERROR_MISSING_OBJECT_KEY,
		ERROR_EXPECTED_COLON,
		ERROR_EXPECTED_END_OF_INPUT,
		ERROR_UNEXPECTED_COMMA,
		ERROR_EXPECTED_VALUE,
		ERROR_EXPECTED_NULL,
		ERROR_EXPECTED_FALSE,
		ERROR_EXPECTED_TRUE,
		ERROR_INVALID_NUMBER,
		ERROR_MISSING_EXPONENT,
		ERROR_ILLEGAL_CODEPOINT,
		ERROR_INVALID_UNICODE_ESCAPE,
		ERROR_UNEXPECTED_END_OF_UTF16,
		ERROR_EXPECTED_U,
		ERROR_INVALID_UTF16_TRAIL_SURROGATE,
		ERROR_UNKNOWN_ESCAPE,
		ERROR_INVALID_UTF8,
	};

	namespace internal {
		class ownership {
		public:
			ownership() = delete;
			ownership(const ownership&) = delete;
			void operator=(const ownership&) = delete;
	
			explicit ownership(size_t* p_)
				: p(p_)
			{}
	
			ownership(ownership&& p_)
			: p(p_.p) {
				p_.p = 0;
			}
	
			~ownership() {
				delete[] p;
			}
	
			bool is_valid() const {
				return !!p;
			}
	
		private:
			size_t* p;
		};

		inline const char* get_error_text(error error_code) {
			switch (error_code) {
				case ERROR_NO_ERROR: return "no error";
				case ERROR_OUT_OF_MEMORY: return  "out of memory";
				case ERROR_UNEXPECTED_END: return  "unexpected end of input";
				case ERROR_MISSING_ROOT_ELEMENT: return  "missing root element";
				case ERROR_BAD_ROOT: return  "document root must be object or array";
				case ERROR_EXPECTED_COMMA: return  "expected ,";
				case ERROR_MISSING_OBJECT_KEY: return  "missing object key";
				case ERROR_EXPECTED_COLON: return  "expected :";
				case ERROR_EXPECTED_END_OF_INPUT: return  "expected end of input";
				case ERROR_UNEXPECTED_COMMA: return  "unexpected comma";
				case ERROR_EXPECTED_VALUE: return  "expected value";
				case ERROR_EXPECTED_NULL: return  "expected 'null'";
				case ERROR_EXPECTED_FALSE: return  "expected 'false'";
				case ERROR_EXPECTED_TRUE: return  "expected 'true'";
				case ERROR_INVALID_NUMBER: return "invalid number";
				case ERROR_MISSING_EXPONENT: return  "missing exponent";
				case ERROR_ILLEGAL_CODEPOINT: return  "illegal unprintable codepoint in string";
				case ERROR_INVALID_UNICODE_ESCAPE: return  "invalid character in unicode escape";
				case ERROR_UNEXPECTED_END_OF_UTF16: return  "unexpected end of input during UTF-16 surrogate pair";
				case ERROR_EXPECTED_U: return  "expected \\u";
				case ERROR_INVALID_UTF16_TRAIL_SURROGATE: return  "invalid UTF-16 trail surrogate";
				case ERROR_UNKNOWN_ESCAPE: return  "unknown escape";
				case ERROR_INVALID_UTF8: return  "invalid UTF-8";
			}

			SAJSON_UNREACHABLE();
		}
	}

	/**
	 * Represents the result of a JSON parse: either is_valid() and the document
	 * contains a root value or parse error information is available.
	 *
	 * Note that the document holds a strong reference to any memory allocated:
	 * any mutable copy of the input text and any memory allocated for the
	 * AST data structure.  Thus, the document must not be deallocated while any
	 * \ref value is in use.
	 */
	class document {
	public:
		document(document&& rhs)
			: input(rhs.input)
			, structure(std::move(rhs.structure))
			, root_type(rhs.root_type)
			, root(rhs.root)
			, error_line(rhs.error_line)
			, error_column(rhs.error_column)
			, error_code(rhs.error_code)
			, error_arg(rhs.error_arg)
		{
			// Yikes... but strcpy is okay here because formatted_error is
			// guaranteed to be null-terminated.
			strcpy(formatted_error_message, rhs.formatted_error_message);
			// should rhs's fields be zeroed too?
		}

		/**
		 * Returns true if the document was parsed successfully.
		 * If true, call get_root() to access the document's root value.
		 * If false, call get_error_line(), get_error_column(), and
		 * get_error_message_as_cstring() to see why the parse failed.
		 */
		bool is_valid() const {
			return root_type == TYPE_ARRAY || root_type == TYPE_OBJECT;
		}

		/// If is_valid(), returns the document's root \ref value.
		value get_root() const {
			return value(root_type, root, input.get_data());
		}

		/// If not is_valid(), returns the one-based line number where the parse failed.
		size_t get_error_line() const {
			return error_line;
		}

		/// If not is_valid(), returns the one-based column number where the parse failed.
		size_t get_error_column() const {
			return error_column;
		}

#ifndef SAJSON_NO_STD_STRING
		/// If not is_valid(), returns a std::string indicating why the parse failed.
		std::string get_error_message_as_string() const {
			return formatted_error_message;
		}
#endif

		/// If not is_valid(), returns a null-terminated C string indicating why the parse failed.
		const char* get_error_message_as_cstring() const {
			return formatted_error_message;
		}

		/// \cond INTERNAL

		// WARNING: Internal function which is subject to change
		error _internal_get_error_code() const {
			return error_code;
		}

		// WARNING: Internal function which is subject to change
		int _internal_get_error_argument() const {
			return error_arg;
		}

		// WARNING: Internal function which is subject to change
		const char* _internal_get_error_text() const {
			return internal::get_error_text(error_code);
		}

		// WARNING: Internal function exposed only for high-performance language bindings.
		type _internal_get_root_type() const {
			return root_type;
		}

		// WARNING: Internal function exposed only for high-performance language bindings.
		const size_t* _internal_get_root() const {
			return root;
		}

		// WARNING: Internal function exposed only for high-performance language bindings.
		const mutable_string_view& _internal_get_input() const {
			return input;
		}

		/// \endcond

	private:
		document(const document&) = delete;
		void operator=(const document&) = delete;

		explicit document(const mutable_string_view& input_, internal::ownership&& structure_, type root_type_, const size_t* root_)
			: input(input_)
			, structure(std::move(structure_))
			, root_type(root_type_)
			, root(root_)
			, error_line(0)
			, error_column(0)
			, error_code(ERROR_NO_ERROR)
			, error_arg(0)
		{
			formatted_error_message[0] = 0;
		}

		explicit document(const mutable_string_view& input_, size_t error_line_, size_t error_column_, const error error_code_, int error_arg_)
			: input(input_)
			, structure(0)
			, root_type(TYPE_NULL)
			, root(0)
			, error_line(error_line_)
			, error_column(error_column_)
			, error_code(error_code_)
			, error_arg(error_arg_)
		{
			formatted_error_message[ERROR_BUFFER_LENGTH - 1] = 0;
			int written = has_significant_error_arg()
				? SAJSON_snprintf(formatted_error_message, ERROR_BUFFER_LENGTH - 1, "%s: %d", _internal_get_error_text(), error_arg)
				: SAJSON_snprintf(formatted_error_message, ERROR_BUFFER_LENGTH - 1, "%s", _internal_get_error_text());
			(void)written;
			assert(written >= 0 && written < ERROR_BUFFER_LENGTH);
		}

		bool has_significant_error_arg() const {
			return error_code == ERROR_ILLEGAL_CODEPOINT;
		}

		mutable_string_view input;
		internal::ownership structure;
		const type root_type;
		const size_t* const root;
		const size_t error_line;
		const size_t error_column;
		const error error_code;
		const int error_arg;

		enum { ERROR_BUFFER_LENGTH = 128 };
		char formatted_error_message[ERROR_BUFFER_LENGTH];

		template<typename AllocationStrategy, typename StringType>
		friend document parse(const AllocationStrategy& strategy, const StringType& string);
		template<typename Allocator>
		friend class parser;
	};

	/** Parse document with single allocation */
	document parse_single_allocation(const mutable_string_view& json);

	/** Parse document with dynamic grow allocation (use less memory)  */
	document parse_dynamic_allocation(const mutable_string_view& json);

	/** Parse document with fixed buffer (zero allocation) */
	document parse_bounded_allocation(
		const mutable_string_view& json,
		size_t* existing_buffer, size_t size_in_words);
}

