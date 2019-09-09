#pragma once
#include <string>
#include "../Exceptions/LogicException.hpp"
#include "./OutputStreamBase.hpp"

namespace cpv::extensions {
	/** Write data to stream, must keep stream live until future resolved */
	static inline seastar::future<> writeAll(
		OutputStreamBase& stream, Packet&& data) {
		return stream.write(std::move(data));
	}
	
	/** Write data to stream, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, Packet&& data) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_exception_future<>(LogicException(
				CPV_CODEINFO, "write to null stream"));
		}
		return stream->write(std::move(data));
	}
	
	/** Write string (rvalue) to stream, must keep stream live until future resolved */
	seastar::future<> writeAll(OutputStreamBase& stream, std::string&& str);
	
	/** Write string (rvalue) to stream, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, std::string&& str) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_exception_future<>(LogicException(
				CPV_CODEINFO, "write to null stream"));
		}
		return writeAll(*stream.get(), std::move(str));
	}
	
	/** Write string view to stream, must keep stream live until future resolved */
	seastar::future<> writeAll(OutputStreamBase& stream, std::string_view str);
	
	/** Write string view to stream, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, std::string_view str) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_exception_future<>(LogicException(
				CPV_CODEINFO, "write to null stream"));
		}
		return writeAll(*stream.get(), str);
	}
	
	/** Write constant c string to stream, must keep stream live until future resolved */
	template <std::size_t Size>
	static inline seastar::future<> writeAll(OutputStreamBase& stream, const char(&str)[Size]) {
		static_assert(Size > 0, "size of c string should not be 0");
		return writeAll(stream, std::string_view(str, Size-1));
	}
	
	/** Write constant c string to stream, must keep stream live until future resolved */
	template <std::size_t Size, class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, const char(&str)[Size]) {
		static_assert(Size > 0, "size of c string should not be 0");
		return writeAll(stream, std::string_view(str, Size-1));
	}
}

