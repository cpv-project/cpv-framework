#pragma once
#include <string>
#include "../Exceptions/LogicException.hpp"
#include "./OutputStreamBase.hpp"

namespace cpv::extensions {
	/** Write string (rvalue) to stream, must keep stream live until future resolved */
	seastar::future<> writeAll(OutputStreamBase& stream, std::string&& str);
	
	/** Write string (rvalue) to stream, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_same_v<
		decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, std::string&& str) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_exception_future<>(LogicException(
				CPV_CODEINFO, "write to null stream"));
		}
		return writeAll(*stream.get(), std::move(str));
	}
	
	/** Write string view to stream, must keep stream live until future resolved */
	seastar::future<> writeAll(OutputStreamBase& stream, const std::string_view& str);
	
	/** Write string view to stream, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_same_v<
		decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, const std::string_view& str) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_exception_future<>(LogicException(
				CPV_CODEINFO, "write to null stream"));
		}
		return writeAll(*stream.get(), str);
	}
	
	/** Write constant c string to stream, must keep stream live until future resolved */
	static inline seastar::future<> writeAll(OutputStreamBase& stream, const char* str) {
		return writeAll(stream, std::string_view(str));
	}
	
	/** Write constant c string to stream, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_same_v<
		decltype(std::declval<T>().get()), OutputStreamBase*>>* = nullptr>
	seastar::future<> writeAll(T& stream, const char* str) {
		return writeAll(stream, std::string_view(str));
	}
}

