#pragma once
#include <string>
#include <utility>
#include "../Utility/Macros.hpp"
#include "./InputStreamBase.hpp"

namespace cpv::extensions {
	/** Read all data from stream and append to given string, must keep stream live until future resolved */
	seastar::future<> readAll(InputStreamBase& stream, std::string& str);

	/** Read all data from stream and append to given string, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), InputStreamBase*>>* = nullptr>
	seastar::future<> readAll(const T& stream, std::string& str) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_ready_future<>();
		}
		return readAll(*stream.get(), str);
	}

	/** Read all data from stream and return it as string, must keep stream live until future resolved */
	seastar::future<std::string> readAll(InputStreamBase& stream);

	/** Read all data from stream and return it as string, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), InputStreamBase*>>* = nullptr>
	seastar::future<std::string> readAll(const T& stream) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_ready_future<std::string>();
		}
		return readAll(*stream.get());
	}

	/** Read all data from stream and return it as buffer, must keep stream live until future resolved */
	seastar::future<seastar::temporary_buffer<char>> readAllAsBuffer(InputStreamBase& stream);

	/** Read all data from stream and return it as buffer, must keep stream live until future resolved */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), InputStreamBase*>>* = nullptr>
	seastar::future<seastar::temporary_buffer<char>> readAllAsBuffer(const T& stream) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_ready_future<seastar::temporary_buffer<char>>();
		}
		return readAllAsBuffer(*stream.get());
	}
}

