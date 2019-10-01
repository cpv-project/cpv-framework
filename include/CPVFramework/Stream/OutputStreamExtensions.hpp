#pragma once
#include <string>
#include "../Exceptions/LogicException.hpp"
#include "./OutputStreamBase.hpp"

namespace cpv::extensions {
	/** Write string view to stream, must keep stream live until future resolved */
	static inline seastar::future<> writeAll(
		OutputStreamBase& stream, std::string_view str) {
		return stream.write(Packet(str));
	}

	/** Write string view to stream, must keep stream live until future resolved */
	static inline seastar::future<> writeAll(
		OutputStreamBase& stream, seastar::temporary_buffer<char>&& buf) {
		return stream.write(Packet(std::move(buf)));
	}

	/** Write data to stream, must keep stream live until future resolved */
	static inline seastar::future<> writeAll(
		OutputStreamBase& stream, Packet&& data) {
		return stream.write(std::move(data));
	}

	/** Write constant c string to stream, must keep stream live until future resolved */
	template <std::size_t Size>
	seastar::future<> writeAll(OutputStreamBase& stream, const char(&str)[Size]) {
		static_assert(Size > 0, "size of c string should not be 0");
		return writeAll(stream, std::string_view(str, Size - 1));
	}

	/** Write string (rvalue) to stream, must keep stream live until future resolved */
	seastar::future<> writeAll(OutputStreamBase& stream, std::string&& str);

	/** Write string (lvalue) to stream, must keep stream live until future resolved */
	static inline seastar::future<> writeAll(OutputStreamBase& stream, const std::string& str) {
		return writeAll(stream, std::string(str));
	}

	/** Write data to stream, must keep stream live until future resolved */
	template <class T, class TData,
		std::enable_if_t<std::is_convertible_v<
			decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, TData&& data) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_exception_future<>(LogicException(
				CPV_CODEINFO, "write to null stream"));
		}
		return writeAll(*stream.get(), std::forward<TData>(data));
	}
}

