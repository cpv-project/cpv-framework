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

#include <CPVFramework/Serialize/JsonDeserializer.sajson.hpp>

namespace sajson {
	/// Allocation policy that allocates one large buffer guaranteed to hold the
	/// resulting AST.  This allocation policy is the fastest since it requires
	/// no conditionals to see if more memory must be allocated.
	class single_allocation {
	public:
		/// \cond INTERNAL

		class stack_head {
		public:
			stack_head(stack_head&& other)
				: stack_bottom(other.stack_bottom)
				, stack_top(other.stack_top)
			{}

			bool push(size_t element) {
				*stack_top++ = element;
				return true;
			}

			size_t* reserve(size_t amount, bool* success) {
				size_t* rv = stack_top;
				stack_top += amount;
				*success = true;
				return rv;
			}

			// The compiler does not see the stack_head (stored in a local)
			// and the allocator (stored as a field) have the same stack_bottom
			// values, so it does a bit of redundant work.
			// So there's a microoptimization available here: introduce a type
			// "stack_mark" and make it polymorphic on the allocator.  For
			// single_allocation, it merely needs to be a single pointer.

			void reset(size_t new_top) {
				stack_top = stack_bottom + new_top;
			}

			size_t get_size() {
				return stack_top - stack_bottom;
			}

			size_t* get_top() {
				return stack_top;
			}

			size_t* get_pointer_from_offset(size_t offset) {
				return stack_bottom + offset;
			}

		private:
			stack_head() = delete;
			stack_head(const stack_head&) = delete;
			void operator=(const stack_head&) = delete;

			explicit stack_head(size_t* base)
				: stack_bottom(base)
				, stack_top(base)
			{}

			size_t* const stack_bottom;
			size_t* stack_top;

			friend class single_allocation;
		};

		class allocator {
		public:
			allocator() = delete;
			allocator(const allocator&) = delete;
			void operator=(const allocator&) = delete;

			explicit allocator(size_t* buffer, size_t input_size, bool should_deallocate_)
				: structure(buffer)
				, structure_end(buffer ? buffer + input_size : 0)
				, write_cursor(structure_end)
				, should_deallocate(should_deallocate_)
			{}

			explicit allocator(std::nullptr_t)
				: structure(0)
				, structure_end(0)
				, write_cursor(0)
				, should_deallocate(false)
			{}

			allocator(allocator&& other)
				: structure(other.structure)
				, structure_end(other.structure_end)
				, write_cursor(other.write_cursor)
				, should_deallocate(other.should_deallocate)
			{
				other.structure = 0;
				other.structure_end = 0;
				other.write_cursor = 0;
				other.should_deallocate = false;
			}

			~allocator() {
				if (should_deallocate) {
					delete[] structure;
				}
			}

			stack_head get_stack_head(bool* success) {
				*success = true;
				return stack_head(structure);
			}

			size_t get_write_offset() {
				return structure_end - write_cursor;
			}

			size_t* get_write_pointer_of(size_t v) {
				return structure_end - v;
			}

			size_t* reserve(size_t size, bool* success) {
				*success = true;
				write_cursor -= size;
				return write_cursor;
			}

			size_t* get_ast_root() {
				return write_cursor;
			}

			internal::ownership transfer_ownership() {
				auto p = structure;
				structure = 0;
				structure_end = 0;
				write_cursor = 0;
				if (should_deallocate) {
					return internal::ownership(p);
				} else {
					return internal::ownership(0);
				}
			}

		private:
			size_t* structure;
			size_t* structure_end;
			size_t* write_cursor;
			bool should_deallocate;
		};

		/// \endcond

		/// Allocate a single worst-case AST buffer with one word per byte in
		/// the input document.
		single_allocation()
			: has_existing_buffer(false)
			, existing_buffer(0)
			, existing_buffer_size(0)
		{}

		/// Write the AST into an existing buffer.  Will fail with an out of
		/// memory error if the buffer is not guaranteed to be big enough for
		/// the document.  The caller must guarantee the memory is valid for
		/// the duration of the parse and the AST traversal.
		single_allocation(size_t* existing_buffer_, size_t size_in_words)
			: has_existing_buffer(true)
			, existing_buffer(existing_buffer_)
			, existing_buffer_size(size_in_words)
		{}

		/// Convenience wrapper for single_allocation(size_t*, size_t) that
		/// automatically infers the length of a given array.
		template<size_t N>
		explicit single_allocation(size_t (&existing_buffer_)[N])
			: single_allocation(existing_buffer_, N)
		{}

		/// \cond INTERNAL

		allocator make_allocator(size_t input_document_size_in_bytes, bool* succeeded) const {
			if (has_existing_buffer) {
				if (existing_buffer_size < input_document_size_in_bytes) {
					*succeeded = false;
					return allocator(nullptr);
				}
				*succeeded = true;
				return allocator(existing_buffer, input_document_size_in_bytes, false);
			} else {
				size_t* buffer = new(std::nothrow) size_t[input_document_size_in_bytes];
				if (!buffer) {
					*succeeded = false;
					return allocator(nullptr);
				}
				*succeeded = true;
				return allocator(buffer, input_document_size_in_bytes, true);
			}
		}

		/// \endcond

	private:
		bool has_existing_buffer;
		size_t* existing_buffer;
		size_t existing_buffer_size;
	};

	/// Allocation policy that uses dynamically-growing buffers for both the
	/// parse stack and the AST.  This allocation policy minimizes peak memory
	/// usage at the cost of some allocation and copying churn.
	class dynamic_allocation {
	public:
		/// \cond INTERNAL

		class stack_head {
		public:
			stack_head(stack_head&& other)
				: stack_top(other.stack_top)
				, stack_bottom(other.stack_bottom)
				, stack_limit(other.stack_limit)
			{
				other.stack_top = 0;
				other.stack_bottom = 0;
				other.stack_limit = 0;
			}

			~stack_head() {
				delete[] stack_bottom;
			}

			bool push(size_t element) {
				if (can_grow(1)) {
					*stack_top++ = element;
					return true;
				} else {
					return false;
				}
			}

			size_t* reserve(size_t amount, bool* success) {
				if (can_grow(amount)) {
					size_t* rv = stack_top;
					stack_top += amount;
					*success = true;
					return rv;
				} else {
					*success = false;
					return 0;
				}
			}

			void reset(size_t new_top) {
				stack_top = stack_bottom + new_top;
			}

			size_t get_size() {
				return stack_top - stack_bottom;
			}

			size_t* get_top() {
				return stack_top;
			}

			size_t* get_pointer_from_offset(size_t offset) {
				return stack_bottom + offset;
			}

		private:
			stack_head(const stack_head&) = delete;
			void operator=(const stack_head&) = delete;

			explicit stack_head(size_t initial_capacity, bool* success) {
				assert(initial_capacity);
				stack_bottom = new(std::nothrow) size_t[initial_capacity];
				stack_top = stack_bottom;
				if (stack_bottom) {
					stack_limit = stack_bottom + initial_capacity;
				} else {
					stack_limit = 0;
				}
				*success = !!stack_bottom;
			}

			bool can_grow(size_t amount) {
				if (SAJSON_LIKELY(amount <= static_cast<size_t>(stack_limit - stack_top))) {
					return true;
				}

				size_t current_size = stack_top - stack_bottom;
				size_t old_capacity = stack_limit - stack_bottom;
				size_t new_capacity = old_capacity * 2;
				while (new_capacity < amount + current_size) {
					new_capacity *= 2;
				}
				size_t* new_stack = new(std::nothrow) size_t[new_capacity];
				if (!new_stack) {
					stack_top = 0;
					stack_bottom = 0;
					stack_limit = 0;
					return false;
				}

				memcpy(new_stack, stack_bottom, current_size * sizeof(size_t));
				delete[] stack_bottom;
				stack_top = new_stack + current_size;
				stack_bottom = new_stack;
				stack_limit = stack_bottom + new_capacity;
				return true;
			}

			size_t* stack_top; // stack grows up: stack_top >= stack_bottom
			size_t* stack_bottom;
			size_t* stack_limit;

			friend class dynamic_allocation;
		};

		class allocator {
		public:
			allocator() = delete;
			allocator(const allocator&) = delete;
			void operator=(const allocator&) = delete;

			explicit allocator(size_t* buffer_, size_t current_capacity, size_t initial_stack_capacity_)
				: ast_buffer_bottom(buffer_)
				, ast_buffer_top(buffer_ + current_capacity)
				, ast_write_head(ast_buffer_top)
				, initial_stack_capacity(initial_stack_capacity_)
			{}

			explicit allocator(std::nullptr_t)
				: ast_buffer_bottom(0)
				, ast_buffer_top(0)
				, ast_write_head(0)
				, initial_stack_capacity(0)
			{}

			allocator(allocator&& other)
				: ast_buffer_bottom(other.ast_buffer_bottom)
				, ast_buffer_top(other.ast_buffer_top)
				, ast_write_head(other.ast_write_head)
				, initial_stack_capacity(other.initial_stack_capacity)
			{
				other.ast_buffer_bottom = 0;
				other.ast_buffer_top = 0;
				other.ast_write_head = 0;
			}

			~allocator() {
				delete[] ast_buffer_bottom;
			}

			stack_head get_stack_head(bool* success) {
				return stack_head(initial_stack_capacity, success);
			}

			size_t get_write_offset() {
				return ast_buffer_top - ast_write_head;
			}

			size_t* get_write_pointer_of(size_t v) {
				return ast_buffer_top - v;
			}

			size_t* reserve(size_t size, bool* success) {
				if (can_grow(size)) {
					ast_write_head -= size;
					*success = true;
					return ast_write_head;
				} else {
					*success = false;
					return 0;
				}
			}

			size_t* get_ast_root() {
				return ast_write_head;
			}

			internal::ownership transfer_ownership() {
				auto p = ast_buffer_bottom;
				ast_buffer_bottom = 0;
				ast_buffer_top = 0;
				ast_write_head = 0;
				return internal::ownership(p);
			}

		private:
			bool can_grow(size_t amount) {
				if (SAJSON_LIKELY(amount <= static_cast<size_t>(ast_write_head - ast_buffer_bottom))) {
					return true;
				}
				size_t current_capacity = ast_buffer_top - ast_buffer_bottom;

				size_t current_size = ast_buffer_top - ast_write_head;
				size_t new_capacity = current_capacity * 2;
				while (new_capacity < amount + current_size) {
					new_capacity *= 2;
				}

				size_t* old_buffer = ast_buffer_bottom;
				size_t* new_buffer = new(std::nothrow) size_t[new_capacity];
				if (!new_buffer) {
					ast_buffer_bottom = 0;
					ast_buffer_top = 0;
					ast_write_head = 0;
					return false;
				}

				size_t* old_write_head = ast_write_head;
				ast_buffer_bottom = new_buffer;
				ast_buffer_top = new_buffer + new_capacity;
				ast_write_head = ast_buffer_top - current_size;
				memcpy(ast_write_head, old_write_head, current_size * sizeof(size_t));
				delete[] old_buffer;

				return true;
			}

			size_t* ast_buffer_bottom; // base address of the ast buffer - it grows down
			size_t* ast_buffer_top;
			size_t* ast_write_head;
			size_t initial_stack_capacity;
		};

		/// \endcond

		/// Creates a dynamic_allocation policy with the given initial AST
		/// and stack buffer sizes.
		dynamic_allocation(size_t initial_ast_capacity_ = 0, size_t initial_stack_capacity_ = 0)
			: initial_ast_capacity(initial_ast_capacity_)
			, initial_stack_capacity(initial_stack_capacity_)
		{}

		/// \cond INTERNAL

		allocator make_allocator(size_t, bool* succeeded) const {
			size_t capacity = initial_ast_capacity;
			if (!capacity) {
				// TODO: guess based on input document size
				capacity = 1024;
			}

			size_t* buffer = new(std::nothrow) size_t[capacity];
			if (!buffer) {
				*succeeded = false;
				return allocator(nullptr);
			}

			size_t stack_capacity = initial_stack_capacity;
			if (!stack_capacity) {
				stack_capacity = 256;
			}

			*succeeded = true;
			return allocator(buffer, capacity, stack_capacity);
		}
	
		/// \endcond

	private:
		size_t initial_ast_capacity;
		size_t initial_stack_capacity;
	};

	/// Allocation policy that attempts to fit the parsed AST into an existing
	/// memory buffer.  This allocation policy is useful when using sajson in
	/// a zero-allocation context or when there are constraints on the amount
	// of memory that can be used.
	class bounded_allocation {
	public:
		/// \cond INTERNAL

		class allocator;

		class stack_head {
		public:
			stack_head(stack_head&& other)
				: source_allocator(other.source_allocator)
			{
				other.source_allocator = 0;
			}

			bool push(size_t element) {
				if (SAJSON_LIKELY(source_allocator->can_grow(1))) {
					*(source_allocator->stack_top)++ = element;
					return true;
				} else {
					return false;
				}
			}

			size_t* reserve(size_t amount, bool* success) {
				if (SAJSON_LIKELY(source_allocator->can_grow(amount))) {
					size_t* rv = source_allocator->stack_top;
					source_allocator->stack_top += amount;
					*success = true;
					return rv;
				} else {
					*success = false;
					return 0;
				}
			}

			void reset(size_t new_top) {
				source_allocator->stack_top = source_allocator->structure + new_top;
			}

			size_t get_size() {
				return source_allocator->stack_top - source_allocator->structure;
			}

			size_t* get_top() {
				return source_allocator->stack_top;
			}

			size_t* get_pointer_from_offset(size_t offset) {
				return source_allocator->structure + offset;
			}

		private:
			stack_head(const stack_head&) = delete;
			void operator=(const stack_head&) = delete;

			explicit stack_head(allocator* source_allocator_)
				: source_allocator(source_allocator_)
			{}

			allocator* source_allocator;

			friend class bounded_allocation;
		};

		class allocator {
		public:
			allocator() = delete;
			allocator(const allocator&) = delete;
			void operator=(const allocator&) = delete;

			explicit allocator(size_t* existing_buffer, size_t existing_buffer_size)
				: structure(existing_buffer)
				, structure_end(existing_buffer + existing_buffer_size)
				, write_cursor(structure_end)
				, stack_top(structure)
			{}

			allocator(allocator&& other)
				: structure(other.structure)
				, structure_end(other.structure_end)
				, write_cursor(other.write_cursor)
				, stack_top(other.stack_top)
			{
				other.structure = 0;
				other.structure_end = 0;
				other.write_cursor = 0;
				other.stack_top = 0;
			}

			stack_head get_stack_head(bool* success) {
				*success = true;
				return stack_head(this);
			}

			size_t get_write_offset() {
				return structure_end - write_cursor;
			}

			size_t* get_write_pointer_of(size_t v) {
				return structure_end - v;
			}

			size_t* reserve(size_t size, bool* success) {
				if (can_grow(size)) {
					write_cursor -= size;
					*success = true;
					return write_cursor;
				} else {
					*success = false;
					return 0;
				}
			}

			size_t* get_ast_root() {
				return write_cursor;
			}

			internal::ownership transfer_ownership() {
				structure = 0;
				structure_end = 0;
				write_cursor = 0;
				return internal::ownership(0);
			}

		private:
			bool can_grow(size_t amount) {
				// invariant: stack_top <= write_cursor
				// thus: write_cursor - stack_top is positive
				return static_cast<size_t>(write_cursor - stack_top) >= amount;
			}

			size_t* structure;
			size_t* structure_end;
			size_t* write_cursor;
			size_t* stack_top;

			friend class bounded_allocation;
		};

		/// \endcond

		/// Uses an existing buffer to hold the parsed AST, if it fits.  The
		/// specified buffer must not be deallocated until after the document
		/// is parsed and the AST traversed.
		bounded_allocation(size_t* existing_buffer_, size_t size_in_words)
			: existing_buffer(existing_buffer_)
			, existing_buffer_size(size_in_words)
		{}

		/// Convenience wrapper for bounded_allocation(size_t*, size) that
		/// automatically infers the size of the given array.
		template<size_t N>
		explicit bounded_allocation(size_t (&existing_buffer_)[N])
			: bounded_allocation(existing_buffer_, N)
		{}

		/// \cond INTERNAL

		allocator make_allocator(size_t, bool* succeeded) const {
			*succeeded = true;
			return allocator(existing_buffer, existing_buffer_size);
		}

		/// \endcond

	private:
		size_t* existing_buffer;
		size_t existing_buffer_size;
	};

	// I thought about putting parser in the internal namespace but I don't
	// want to indent it further...
	/// \cond INTERNAL
	template<typename Allocator>
	class parser {
	public:
		parser(const mutable_string_view& msv, Allocator&& allocator_)
			: input(msv)
			, input_end(input.get_data() + input.length())
			, allocator(std::move(allocator_))
			, root_type(TYPE_NULL)
			, error_line(0)
			, error_column(0)
		{}

		document get_document() {
			if (parse()) {
				size_t* ast_root = allocator.get_ast_root();
				return document(input, allocator.transfer_ownership(), root_type, ast_root);
			} else {
				return document(input, error_line, error_column, error_code, error_arg);
			}
		}

	private:
		struct error_result {
			operator bool() const {
				return false;
			}
			operator char*() const {
				return 0;
			}
		};

		bool at_eof(const char* p) {
			return p == input_end;
		}

		char* skip_whitespace(char* p) {
			// There is an opportunity to make better use of superscalar
			// hardware here* but if someone cares about JSON parsing
			// performance the first thing they do is minify, so prefer
			// to optimize for code size here.
			// * https://github.com/chadaustin/Web-Benchmarks/blob/master/json/third-party/pjson/pjson.h#L1873
			for (;;) {
				if (SAJSON_UNLIKELY(p == input_end)) {
					return 0;
				} else if (internal::is_whitespace(*p)) {
					++p;
				} else {
					return p;
				}
			}
		}

		error_result oom(char* p) {
			return make_error(p, ERROR_OUT_OF_MEMORY);
		}

		error_result unexpected_end() {
			return make_error(0, ERROR_UNEXPECTED_END);
		}

		error_result unexpected_end(char* p) {
			return make_error(p, ERROR_UNEXPECTED_END);
		}

		error_result make_error(char* p, error code, int arg = 0) {
			if (!p) {
				p = input_end;
			}

			error_line = 1;
			error_column = 1;

			char* c = input.get_data();
			while (c < p) {
				if (*c == '\r') {
					if (c + 1 < p && c[1] == '\n') {
						++error_line;
						error_column = 1;
						++c;
					} else {
						++error_line;
						error_column = 1;
					}
				} else if (*c == '\n') {
					++error_line;
					error_column = 1;
				} else {
					// TODO: count UTF-8 characters
					++error_column;
				}
				++c;
			}

			error_code = code;
			error_arg = arg;
			return error_result();
		}

		bool parse() {
			using namespace internal;

			// p points to the character currently being parsed
			char* p = input.get_data();

			bool success;
			auto stack = allocator.get_stack_head(&success);
			if (SAJSON_UNLIKELY(!success)) {
				return oom(p);
			}

			p = skip_whitespace(p);
			if (SAJSON_UNLIKELY(!p)) {
				return make_error(p, ERROR_MISSING_ROOT_ELEMENT);
			}

			// current_base is an offset to the first element of the current structure (object or array)
			size_t current_base = stack.get_size();
			type current_structure_type;
			if (*p == '[') {
				current_structure_type = TYPE_ARRAY;
				bool s = stack.push(make_element(current_structure_type, ROOT_MARKER));
				if (SAJSON_UNLIKELY(!s)) {
					return oom(p);
				}
				goto array_close_or_element;
			} else if (*p == '{') {
				current_structure_type = TYPE_OBJECT;
				bool s = stack.push(make_element(current_structure_type, ROOT_MARKER));
				if (SAJSON_UNLIKELY(!s)) {
					return oom(p);
				}
				goto object_close_or_element;
			} else {
				return make_error(p, ERROR_BAD_ROOT);
			}

			// BEGIN STATE MACHINE

			size_t pop_element; // used as an argument into the `pop` routine

			if (0) { // purely for structure

			// ASSUMES: byte at p SHOULD be skipped
			array_close_or_element:
				p = skip_whitespace(p + 1);
				if (SAJSON_UNLIKELY(!p)) {
					return unexpected_end();
				}
				if (*p == ']') {
					goto pop_array;
				} else {
					goto next_element;
				}
				SAJSON_UNREACHABLE();

			// ASSUMES: byte at p SHOULD be skipped
			object_close_or_element:
				p = skip_whitespace(p + 1);
				if (SAJSON_UNLIKELY(!p)) {
					return unexpected_end();
				}
				if (*p == '}') {
					goto pop_object;
				} else {
					goto object_key;
				}
				SAJSON_UNREACHABLE();

			// ASSUMES: byte at p SHOULD NOT be skipped
			structure_close_or_comma:
				p = skip_whitespace(p);
				if (SAJSON_UNLIKELY(!p)) {
					return unexpected_end();
				}

				if (current_structure_type == TYPE_ARRAY) {
					if (*p == ']') {
						goto pop_array;
					} else {
						if (SAJSON_UNLIKELY(*p != ',')) {
							return make_error(p, ERROR_EXPECTED_COMMA);
						}
						++p;
						goto next_element;
					}
				} else {
					assert(current_structure_type == TYPE_OBJECT);
					if (*p == '}') {
						goto pop_object;
					} else {
						if (SAJSON_UNLIKELY(*p != ',')) {
							return make_error(p, ERROR_EXPECTED_COMMA);
						}
						++p;
						goto object_key;
					}
				}
				SAJSON_UNREACHABLE();

			// ASSUMES: *p == '}'
			pop_object: {
				++p;
				size_t* base_ptr = stack.get_pointer_from_offset(current_base);
				pop_element = *base_ptr;
				if (SAJSON_UNLIKELY(!install_object(base_ptr + 1, stack.get_top()))) {
					return oom(p);
				}
				goto pop;
			}

			// ASSUMES: *p == ']'
			pop_array: {
				++p;
				size_t* base_ptr = stack.get_pointer_from_offset(current_base);
				pop_element = *base_ptr;
				if (SAJSON_UNLIKELY(!install_array(base_ptr + 1, stack.get_top()))) {
					return oom(p);
				}
				goto pop;
			}

			// ASSUMES: byte at p SHOULD NOT be skipped
			object_key: {
				p = skip_whitespace(p);
				if (SAJSON_UNLIKELY(!p)) {
					return unexpected_end();
				}
				if (SAJSON_UNLIKELY(*p != '"')) {
					return make_error(p, ERROR_MISSING_OBJECT_KEY);
				}
				bool success_;
				size_t* out = stack.reserve(2, &success_);
				if (SAJSON_UNLIKELY(!success_)) {
					return oom(p);
				}
				p = parse_string(p, out);
				if (SAJSON_UNLIKELY(!p)) {
					return false;
				}
				p = skip_whitespace(p);
				if (SAJSON_UNLIKELY(!p || *p != ':')) {
					return make_error(p, ERROR_EXPECTED_COLON);
				}
				++p;
				goto next_element;
			}

			// ASSUMES: byte at p SHOULD NOT be skipped
			next_element:
				p = skip_whitespace(p);
				if (SAJSON_UNLIKELY(!p)) {
					return unexpected_end();
				}

				type value_type_result;
				switch (*p) {
					case 0:
						return unexpected_end(p);
					case 'n':
						p = parse_null(p);
						if (!p) {
							return false;
						}
						value_type_result = TYPE_NULL;
						break;
					case 'f':
						p = parse_false(p);
						if (!p) {
							return false;
						}
						value_type_result = TYPE_FALSE;
						break;
					case 't':
						p = parse_true(p);
						if (!p) {
							return false;
						}
						value_type_result = TYPE_TRUE;
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '-': {
						auto result = parse_number(p);
						p = result.first;
						if (!p) {
							return false;
						}
						value_type_result = result.second;
						break;
					}
					case '"': {
						bool success_;
						size_t* string_tag = allocator.reserve(2, &success_);
						if (SAJSON_UNLIKELY(!success_)) {
							return oom(p);
						}
						p = parse_string(p, string_tag);
						if (!p) {
							return false;
						}
						value_type_result = TYPE_STRING;
						break;
					}

					case '[': {
						size_t previous_base = current_base;
						current_base = stack.get_size();
						bool s = stack.push(make_element(current_structure_type, previous_base));
						if (SAJSON_UNLIKELY(!s)) {
							return oom(p);
						}
						current_structure_type = TYPE_ARRAY;
						goto array_close_or_element;
					}
					case '{': {
						size_t previous_base = current_base;
						current_base = stack.get_size();
						bool s = stack.push(make_element(current_structure_type, previous_base));
						if (SAJSON_UNLIKELY(!s)) {
							return oom(p);
						}
						current_structure_type = TYPE_OBJECT;
						goto object_close_or_element;
					}
					pop: {
						size_t parent = get_element_value(pop_element);
						if (parent == ROOT_MARKER) {
							root_type = current_structure_type;
							p = skip_whitespace(p);
							if (SAJSON_UNLIKELY(p)) {
								return make_error(p, ERROR_EXPECTED_END_OF_INPUT);
							}
							return true;
						}
						stack.reset(current_base);
						current_base = parent;
						value_type_result = current_structure_type;
						current_structure_type = get_element_type(pop_element);
						break;
					}

					case ',':
						return make_error(p, ERROR_UNEXPECTED_COMMA);
					default:
						return make_error(p, ERROR_EXPECTED_VALUE);
				}

				bool s = stack.push(make_element(
					value_type_result,
					allocator.get_write_offset()));
				if (SAJSON_UNLIKELY(!s)) {
					return oom(p);
				}

				goto structure_close_or_comma;
			}

			SAJSON_UNREACHABLE();
		}

		bool has_remaining_characters(char* p, ptrdiff_t remaining) {
			return input_end - p >= remaining;
		}

		char* parse_null(char* p) {
			if (SAJSON_UNLIKELY(!has_remaining_characters(p, 4))) {
				make_error(p, ERROR_UNEXPECTED_END);
				return 0;
			}
			char p1 = p[1];
			char p2 = p[2];
			char p3 = p[3];
			if (SAJSON_UNLIKELY(p1 != 'u' || p2 != 'l' || p3 != 'l')) {
				make_error(p, ERROR_EXPECTED_NULL);
				return 0;
			}
			return p + 4;
		}

		char* parse_false(char* p) {
			if (SAJSON_UNLIKELY(!has_remaining_characters(p, 5))) {
				return make_error(p, ERROR_UNEXPECTED_END);
			}
			char p1 = p[1];
			char p2 = p[2];
			char p3 = p[3];
			char p4 = p[4];
			if (SAJSON_UNLIKELY(p1 != 'a' || p2 != 'l' || p3 != 's' || p4 != 'e')) {
				return make_error(p, ERROR_EXPECTED_FALSE);
			}
			return p + 5;
		}

		char* parse_true(char* p) {
			if (SAJSON_UNLIKELY(!has_remaining_characters(p, 4))) {
				return make_error(p, ERROR_UNEXPECTED_END);
			}
			char p1 = p[1];
			char p2 = p[2];
			char p3 = p[3];
			if (SAJSON_UNLIKELY(p1 != 'r' || p2 != 'u' || p3 != 'e')) {
				return make_error(p, ERROR_EXPECTED_TRUE);
			}
			return p + 4;
		}

		static double pow10(int64_t exponent) {
			if (SAJSON_UNLIKELY(exponent > 308)) {
				return std::numeric_limits<double>::infinity();
			} else if (SAJSON_UNLIKELY(exponent < -323)) {
				return 0.0;
			}
			static const double constants[] = {
				1e-323,1e-322,1e-321,1e-320,1e-319,1e-318,1e-317,1e-316,1e-315,1e-314,
				1e-313,1e-312,1e-311,1e-310,1e-309,1e-308,1e-307,1e-306,1e-305,1e-304,
				1e-303,1e-302,1e-301,1e-300,1e-299,1e-298,1e-297,1e-296,1e-295,1e-294,
				1e-293,1e-292,1e-291,1e-290,1e-289,1e-288,1e-287,1e-286,1e-285,1e-284,
				1e-283,1e-282,1e-281,1e-280,1e-279,1e-278,1e-277,1e-276,1e-275,1e-274,
				1e-273,1e-272,1e-271,1e-270,1e-269,1e-268,1e-267,1e-266,1e-265,1e-264,
				1e-263,1e-262,1e-261,1e-260,1e-259,1e-258,1e-257,1e-256,1e-255,1e-254,
				1e-253,1e-252,1e-251,1e-250,1e-249,1e-248,1e-247,1e-246,1e-245,1e-244,
				1e-243,1e-242,1e-241,1e-240,1e-239,1e-238,1e-237,1e-236,1e-235,1e-234,
				1e-233,1e-232,1e-231,1e-230,1e-229,1e-228,1e-227,1e-226,1e-225,1e-224,
				1e-223,1e-222,1e-221,1e-220,1e-219,1e-218,1e-217,1e-216,1e-215,1e-214,
				1e-213,1e-212,1e-211,1e-210,1e-209,1e-208,1e-207,1e-206,1e-205,1e-204,
				1e-203,1e-202,1e-201,1e-200,1e-199,1e-198,1e-197,1e-196,1e-195,1e-194,
				1e-193,1e-192,1e-191,1e-190,1e-189,1e-188,1e-187,1e-186,1e-185,1e-184,
				1e-183,1e-182,1e-181,1e-180,1e-179,1e-178,1e-177,1e-176,1e-175,1e-174,
				1e-173,1e-172,1e-171,1e-170,1e-169,1e-168,1e-167,1e-166,1e-165,1e-164,
				1e-163,1e-162,1e-161,1e-160,1e-159,1e-158,1e-157,1e-156,1e-155,1e-154,
				1e-153,1e-152,1e-151,1e-150,1e-149,1e-148,1e-147,1e-146,1e-145,1e-144,
				1e-143,1e-142,1e-141,1e-140,1e-139,1e-138,1e-137,1e-136,1e-135,1e-134,
				1e-133,1e-132,1e-131,1e-130,1e-129,1e-128,1e-127,1e-126,1e-125,1e-124,
				1e-123,1e-122,1e-121,1e-120,1e-119,1e-118,1e-117,1e-116,1e-115,1e-114,
				1e-113,1e-112,1e-111,1e-110,1e-109,1e-108,1e-107,1e-106,1e-105,1e-104,
				1e-103,1e-102,1e-101,1e-100,1e-99,1e-98,1e-97,1e-96,1e-95,1e-94,1e-93,
				1e-92,1e-91,1e-90,1e-89,1e-88,1e-87,1e-86,1e-85,1e-84,1e-83,1e-82,1e-81,
				1e-80,1e-79,1e-78,1e-77,1e-76,1e-75,1e-74,1e-73,1e-72,1e-71,1e-70,1e-69,
				1e-68,1e-67,1e-66,1e-65,1e-64,1e-63,1e-62,1e-61,1e-60,1e-59,1e-58,1e-57,
				1e-56,1e-55,1e-54,1e-53,1e-52,1e-51,1e-50,1e-49,1e-48,1e-47,1e-46,1e-45,
				1e-44,1e-43,1e-42,1e-41,1e-40,1e-39,1e-38,1e-37,1e-36,1e-35,1e-34,1e-33,
				1e-32,1e-31,1e-30,1e-29,1e-28,1e-27,1e-26,1e-25,1e-24,1e-23,1e-22,1e-21,
				1e-20,1e-19,1e-18,1e-17,1e-16,1e-15,1e-14,1e-13,1e-12,1e-11,1e-10,1e-9,
				1e-8,1e-7,1e-6,1e-5,1e-4,1e-3,1e-2,1e-1,1e0,1e1,1e2,1e3,1e4,1e5,1e6,1e7,
				1e8,1e9,1e10,1e11,1e12,1e13,1e14,1e15,1e16,1e17,1e18,1e19,1e20,1e21,
				1e22,1e23,1e24,1e25,1e26,1e27,1e28,1e29,1e30,1e31,1e32,1e33,1e34,1e35,
				1e36,1e37,1e38,1e39,1e40,1e41,1e42,1e43,1e44,1e45,1e46,1e47,1e48,1e49,
				1e50,1e51,1e52,1e53,1e54,1e55,1e56,1e57,1e58,1e59,1e60,1e61,1e62,1e63,
				1e64,1e65,1e66,1e67,1e68,1e69,1e70,1e71,1e72,1e73,1e74,1e75,1e76,1e77,
				1e78,1e79,1e80,1e81,1e82,1e83,1e84,1e85,1e86,1e87,1e88,1e89,1e90,1e91,
				1e92,1e93,1e94,1e95,1e96,1e97,1e98,1e99,1e100,1e101,1e102,1e103,1e104,
				1e105,1e106,1e107,1e108,1e109,1e110,1e111,1e112,1e113,1e114,1e115,1e116,
				1e117,1e118,1e119,1e120,1e121,1e122,1e123,1e124,1e125,1e126,1e127,1e128,
				1e129,1e130,1e131,1e132,1e133,1e134,1e135,1e136,1e137,1e138,1e139,1e140,
				1e141,1e142,1e143,1e144,1e145,1e146,1e147,1e148,1e149,1e150,1e151,1e152,
				1e153,1e154,1e155,1e156,1e157,1e158,1e159,1e160,1e161,1e162,1e163,1e164,
				1e165,1e166,1e167,1e168,1e169,1e170,1e171,1e172,1e173,1e174,1e175,1e176,
				1e177,1e178,1e179,1e180,1e181,1e182,1e183,1e184,1e185,1e186,1e187,1e188,
				1e189,1e190,1e191,1e192,1e193,1e194,1e195,1e196,1e197,1e198,1e199,1e200,
				1e201,1e202,1e203,1e204,1e205,1e206,1e207,1e208,1e209,1e210,1e211,1e212,
				1e213,1e214,1e215,1e216,1e217,1e218,1e219,1e220,1e221,1e222,1e223,1e224,
				1e225,1e226,1e227,1e228,1e229,1e230,1e231,1e232,1e233,1e234,1e235,1e236,
				1e237,1e238,1e239,1e240,1e241,1e242,1e243,1e244,1e245,1e246,1e247,1e248,
				1e249,1e250,1e251,1e252,1e253,1e254,1e255,1e256,1e257,1e258,1e259,1e260,
				1e261,1e262,1e263,1e264,1e265,1e266,1e267,1e268,1e269,1e270,1e271,1e272,
				1e273,1e274,1e275,1e276,1e277,1e278,1e279,1e280,1e281,1e282,1e283,1e284,
				1e285,1e286,1e287,1e288,1e289,1e290,1e291,1e292,1e293,1e294,1e295,1e296,
				1e297,1e298,1e299,1e300,1e301,1e302,1e303,1e304,1e305,1e306,1e307,1e308
			};
			return constants[exponent + 323];
		}

		std::pair<char*, type> parse_number(char* p) {
			bool negative = false;
			if ('-' == *p) {
				++p;
				negative = true;

				if (SAJSON_UNLIKELY(at_eof(p))) {
					return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
				}
			}

			bool try_double = false;

			int i = 0;
			double d = 0.0; // gcc complains that d might be used uninitialized which isn't true. appease the warning anyway.
			if (*p == '0') {
				++p;
				if (SAJSON_UNLIKELY(at_eof(p))) {
					return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
				}
			} else {
				unsigned char c = *p;
				if (c < '0' || c > '9') {
					return std::make_pair(make_error(p, ERROR_INVALID_NUMBER), TYPE_NULL);
				}

				do {
					++p;
					if (SAJSON_UNLIKELY(at_eof(p))) {
						return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
					}

					unsigned char digit = c - '0';

					if (SAJSON_UNLIKELY(!try_double && i > INT_MAX / 10 - 9)) {
						// TODO: could split this into two loops
						try_double = true;
						d = i;
					}
					if (SAJSON_UNLIKELY(try_double)) {
						d = 10.0 * d + digit;
					} else {
						i = 10 * i + digit;
					}

					c = *p;
				} while (c >= '0' && c <= '9');
			}

			int64_t exponent = 0;

			if ('.' == *p) {
				if (!try_double) {
					try_double = true;
					d = i;
				}
				++p;
				if (SAJSON_UNLIKELY(at_eof(p))) {
					return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
				}
				char c = *p;
				if (c < '0' || c > '9') {
					return std::make_pair(make_error(p, ERROR_INVALID_NUMBER), TYPE_NULL);
				}

				do {
					++p;
					if (SAJSON_UNLIKELY(at_eof(p))) {
						return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
					}
					d = d * 10 + (c - '0');
					// One option to avoid underflow would be to clamp
					// to INT_MIN, but int64 subtraction is cheap and
					// in the absurd case of parsing 2 GB of digits
					// with an extremely high exponent, this will
					// produce accurate results.  Instead, we just
					// leave exponent as int64_t and it will never
					// underflow.
					--exponent;

					c = *p;
				} while (c >= '0' && c <= '9');
			}

			char e = *p;
			if ('e' == e || 'E' == e) {
				if (!try_double) {
					try_double = true;
					d = i;
				}
				++p;
				if (SAJSON_UNLIKELY(at_eof(p))) {
					return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
				}

				bool negativeExponent = false;
				if ('-' == *p) {
					negativeExponent = true;
					++p;
					if (SAJSON_UNLIKELY(at_eof(p))) {
						return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
					}
				} else if ('+' == *p) {
					++p;
					if (SAJSON_UNLIKELY(at_eof(p))) {
						return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
					}
				}

				int exp = 0;

				char c = *p;
				if (SAJSON_UNLIKELY(c < '0' || c > '9')) {
					return std::make_pair(make_error(p, ERROR_MISSING_EXPONENT), TYPE_NULL);
				}
				for (;;) {
					// c guaranteed to be between '0' and '9', inclusive
					unsigned char digit = c - '0';
					if (exp > (INT_MAX - digit) / 10) {
						// The exponent overflowed.  Keep parsing, but
						// it will definitely be out of range when
						// pow10 is called.
						exp = INT_MAX;
					} else {
						exp = 10 * exp + digit;
					}

					++p;
					if (SAJSON_UNLIKELY(at_eof(p))) {
						return std::make_pair(make_error(p, ERROR_UNEXPECTED_END), TYPE_NULL);
					}

					c = *p;
					if (c < '0' || c > '9') {
						break;
					}
				}
				static_assert(-INT_MAX >= INT_MIN, "exp can be negated without loss or UB");
				exponent += (negativeExponent ? -exp : exp);
			}

			if (exponent) {
				assert(try_double);
				// If d is zero but the exponent is huge, don't
				// multiply zero by inf which gives nan.
				if (d != 0.0) {
					d *= pow10(exponent);
				}
			}

			if (negative) {
				if (try_double) {
					d = -d;
				} else {
					i = -i;
				}
			}
			if (try_double) {
				bool success;
				size_t* out = allocator.reserve(double_storage::word_length, &success);
				if (SAJSON_UNLIKELY(!success)) {
					return std::make_pair(oom(p), TYPE_NULL);
				}
				double_storage::store(out, d);
				return std::make_pair(p, TYPE_DOUBLE);
			} else {
				bool success;
				size_t* out = allocator.reserve(integer_storage::word_length, &success);
				if (SAJSON_UNLIKELY(!success)) {
					return std::make_pair(oom(p), TYPE_NULL);
				}
				integer_storage::store(out, i);
				return std::make_pair(p, TYPE_INTEGER);
			}
		}

		bool install_array(size_t* array_base, size_t* array_end) {
			using namespace sajson::internal;

			const size_t length = array_end - array_base;
			bool success;
			size_t* const new_base = allocator.reserve(length + 1, &success);
			if (SAJSON_UNLIKELY(!success)) {
				return false;
			}
			size_t* out = new_base + length + 1;
			size_t* const structure_end = allocator.get_write_pointer_of(0);

			while (array_end > array_base) {
				size_t element = *--array_end;
				type element_type = get_element_type(element);
				size_t element_value = get_element_value(element);
				size_t* element_ptr = structure_end - element_value;
				*--out = make_element(element_type, element_ptr - new_base);
			}
			*--out = length;
			return true;
		}

		bool install_object(size_t* object_base, size_t* object_end) {
			using namespace internal;

			assert((object_end - object_base) % 3 == 0);
			const size_t length_times_3 = object_end - object_base;
#ifndef SAJSON_UNSORTED_OBJECT_KEYS
			std::sort(
				reinterpret_cast<object_key_record*>(object_base),
				reinterpret_cast<object_key_record*>(object_end),
				object_key_comparator(input.get_data()));
#endif

			bool success;
			size_t* const new_base = allocator.reserve(length_times_3 + 1, &success);
			if (SAJSON_UNLIKELY(!success)) {
				return false;
			}
			size_t* out = new_base + length_times_3 + 1;
			size_t* const structure_end = allocator.get_write_pointer_of(0);

			while (object_end > object_base) {
				size_t element = *--object_end;
				type element_type = get_element_type(element);
				size_t element_value = get_element_value(element);
				size_t* element_ptr = structure_end - element_value;

				*--out = make_element(element_type, element_ptr - new_base);
				*--out = *--object_end;
				*--out = *--object_end;
			}
			*--out = length_times_3 / 3;
			return true;
		}

		char* parse_string(char* p, size_t* tag) {
			using namespace internal;

			++p; // "
			size_t start = p - input.get_data();
			char* input_end_local = input_end;
			while (input_end_local - p >= 4) {
				if (!is_plain_string_character(p[0])) { goto found; }
				if (!is_plain_string_character(p[1])) { p += 1; goto found; }
				if (!is_plain_string_character(p[2])) { p += 2; goto found; }
				if (!is_plain_string_character(p[3])) { p += 3; goto found; }
				p += 4;
			}
			for (;;) {
				if (SAJSON_UNLIKELY(p >= input_end_local)) {
					return make_error(p, ERROR_UNEXPECTED_END);
				}

				if (!is_plain_string_character(*p)) {
					break;
				}

				++p;
			}
		found:
			if (SAJSON_LIKELY(*p == '"')) {
				tag[0] = start;
				tag[1] = p - input.get_data();
				*p = '\0';
				return p + 1;
			}

			if (*p >= 0 && *p < 0x20) {
				return make_error(p, ERROR_ILLEGAL_CODEPOINT, static_cast<int>(*p));
			} else {
				// backslash or >0x7f
				return parse_string_slow(p, tag, start);
			}
		}

		char* read_hex(char* p, unsigned& u) {
			unsigned v = 0;
			int i = 4;
			while (i--) {
				unsigned char c = *p++;
				if (c >= '0' && c <= '9') {
					c -= '0';
				} else if (c >= 'a' && c <= 'f') {
					c = c - 'a' + 10;
				} else if (c >= 'A' && c <= 'F') {
					c = c - 'A' + 10;
				} else {
					return make_error(p, ERROR_INVALID_UNICODE_ESCAPE);
				}
				v = (v << 4) + c;
			}

			u = v;
			return p;
		}

		void write_utf8(unsigned codepoint, char*& end) {
			if (codepoint < 0x80) {
				*end++ = codepoint;
			} else if (codepoint < 0x800) {
				*end++ = 0xC0 | (codepoint >> 6);
				*end++ = 0x80 | (codepoint & 0x3F);
			} else if (codepoint < 0x10000) {
				*end++ = 0xE0 | (codepoint >> 12);
				*end++ = 0x80 | ((codepoint >> 6) & 0x3F);
				*end++ = 0x80 | (codepoint & 0x3F);
			} else {
				assert(codepoint < 0x200000);
				*end++ = 0xF0 | (codepoint >> 18);
				*end++ = 0x80 | ((codepoint >> 12) & 0x3F);
				*end++ = 0x80 | ((codepoint >> 6) & 0x3F);
				*end++ = 0x80 | (codepoint & 0x3F);
			}
		}

		char* parse_string_slow(char* p, size_t* tag, size_t start) {
			char* end = p;
			char* input_end_local = input_end;

			for (;;) {
				if (SAJSON_UNLIKELY(p >= input_end_local)) {
					return make_error(p, ERROR_UNEXPECTED_END);
				}

				if (SAJSON_UNLIKELY(*p >= 0 && *p < 0x20)) {
					return make_error(p, ERROR_ILLEGAL_CODEPOINT, static_cast<int>(*p));
				}

				switch (*p) {
					case '"':
						tag[0] = start;
						tag[1] = end - input.get_data();
						*end = '\0';
						return p + 1;

					case '\\':
						++p;
						if (SAJSON_UNLIKELY(p >= input_end_local)) {
							return make_error(p, ERROR_UNEXPECTED_END);
						}

						char replacement;
						switch (*p) {
							case '"': replacement = '"'; goto replace;
							case '\\': replacement = '\\'; goto replace;
							case '/': replacement = '/'; goto replace;
							case 'b': replacement = '\b'; goto replace;
							case 'f': replacement = '\f'; goto replace;
							case 'n': replacement = '\n'; goto replace;
							case 'r': replacement = '\r'; goto replace;
							case 't': replacement = '\t'; goto replace;
							replace:
								*end++ = replacement;
								++p;
								break;
							case 'u': {
								++p;
								if (SAJSON_UNLIKELY(!has_remaining_characters(p, 4))) {
									return make_error(p, ERROR_UNEXPECTED_END);
								}
								unsigned u = 0; // gcc's complaining that this could be used uninitialized. wrong.
								p = read_hex(p, u);
								if (!p) {
									return 0;
								}
								if (u >= 0xD800 && u <= 0xDBFF) {
									if (SAJSON_UNLIKELY(!has_remaining_characters(p, 6))) {
										return make_error(p, ERROR_UNEXPECTED_END_OF_UTF16);
									}
									char p0 = p[0];
									char p1 = p[1];
									if (p0 != '\\' || p1 != 'u') {
										return make_error(p, ERROR_EXPECTED_U);
									}
									p += 2;
									unsigned v = 0; // gcc's complaining that this could be used uninitialized. wrong.
									p = read_hex(p, v);
									if (!p) {
										return p;
									}

									if (v < 0xDC00 || v > 0xDFFF) {
										return make_error(p, ERROR_INVALID_UTF16_TRAIL_SURROGATE);
									}
									u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
								}
								write_utf8(u, end);
								break;
							}
							default:
								return make_error(p, ERROR_UNKNOWN_ESCAPE);
						}
						break;

					default:
						// validate UTF-8
						unsigned char c0 = p[0];
						if (c0 < 128) {
							*end++ = *p++;
						} else if (c0 < 224) {
							if (SAJSON_UNLIKELY(!has_remaining_characters(p, 2))) {
								return unexpected_end(p);
							}
							unsigned char c1 = p[1];
							if (c1 < 128 || c1 >= 192) {
								return make_error(p + 1, ERROR_INVALID_UTF8);
							}
							end[0] = c0;
							end[1] = c1;
							end += 2;
							p += 2;
						} else if (c0 < 240) {
							if (SAJSON_UNLIKELY(!has_remaining_characters(p, 3))) {
								return unexpected_end(p);
							}
							unsigned char c1 = p[1];
							if (c1 < 128 || c1 >= 192) {
								return make_error(p + 1, ERROR_INVALID_UTF8);
							}
							unsigned char c2 = p[2];
							if (c2 < 128 || c2 >= 192) {
								return make_error(p + 2, ERROR_INVALID_UTF8);
							}
							end[0] = c0;
							end[1] = c1;
							end[2] = c2;
							end += 3;
							p += 3;
						} else if (c0 < 248) {
							if (SAJSON_UNLIKELY(!has_remaining_characters(p, 4))) {
								return unexpected_end(p);
							}
							unsigned char c1 = p[1];
							if (c1 < 128 || c1 >= 192) {
								return make_error(p + 1, ERROR_INVALID_UTF8);
							}
							unsigned char c2 = p[2];
							if (c2 < 128 || c2 >= 192) {
								return make_error(p + 2, ERROR_INVALID_UTF8);
							}
							unsigned char c3 = p[3];
							if (c3 < 128 || c3 >= 192) {
								return make_error(p + 3, ERROR_INVALID_UTF8);
							}
							end[0] = c0;
							end[1] = c1;
							end[2] = c2;
							end[3] = c3;
							end += 4;
							p += 4;
						} else {
							return make_error(p, ERROR_INVALID_UTF8);
						}
						break;
				}
			}
		}

		mutable_string_view input;
		char* const input_end;
		Allocator allocator;

		type root_type;
		size_t error_line;
		size_t error_column;
		error error_code;
		int error_arg; // optional argument for the error
	};
	/// \endcond

	/**
	 * Parses a string of JSON bytes into a \ref document, given an allocation
	 * strategy instance.  Any kind of string type is valid as long as a
	 * mutable_string_view can be constructed from it.
	 *
	 * Valid allocation strategies are \ref single_allocation,
	 * \ref dynamic_allocation, and \ref bounded_allocation.
	 *
	 * A \ref document is returned whether or not the parse succeeds: success
	 * state is available by calling document::is_valid().
	 */
	template<typename AllocationStrategy, typename StringType>
	document parse(const AllocationStrategy& strategy, const StringType& string) {
		mutable_string_view input(string);

		bool success;
		auto allocator = strategy.make_allocator(input.length(), &success);
		if (!success) {
			return document(input, 1, 1, ERROR_OUT_OF_MEMORY, 0);
		}

		return parser<typename AllocationStrategy::allocator>(
			input,
			std::move(allocator)
		).get_document();
	}

	/** Parse document with single allocation */
	document parse_single_allocation(const mutable_string_view& json) {
		return parse(single_allocation(), json);
	}

	/** Parse document with dynamic grow allocation (use less memory)  */
	document parse_dynamic_allocation(const mutable_string_view& json) {
		return parse(dynamic_allocation(), json);
	}

	/** Parse document with fixed buffer (zero allocation) */
	document parse_bounded_allocation(
		const mutable_string_view& json,
		size_t* existing_buffer, size_t size_in_words) {
		return parse(bounded_allocation(existing_buffer, size_in_words), json);
	}
}

