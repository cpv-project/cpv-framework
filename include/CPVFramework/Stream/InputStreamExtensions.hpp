#pragma once
#include <utility>
#include "../Utility/Macros.hpp"
#include "../Utility/SharedStringBuilder.hpp"
#include "./InputStreamBase.hpp"

namespace cpv::extensions {
	/**
	 * Read all data from stream and append to given string builder,
	 * must keep stream and string builder alive until future resolved.
	 */
	seastar::future<> readAll(InputStreamBase& stream, SharedStringBuilder& builder);

	/**
	 * Read all data from stream and append to given string builder,
	 * must keep stream and string builder alive until future resolved.
	 */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), InputStreamBase*>>* = nullptr>
	seastar::future<> readAll(const T& stream, SharedStringBuilder& builder) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_ready_future<>();
		}
		return readAll(*stream.get(), builder);
	}

	/**
	 * Read all data from stream and return it as string,
	 * must keep stream live until future resolved.
	 */
	seastar::future<SharedString> readAll(InputStreamBase& stream);

	/**
	 * Read all data from stream and return it as string,
	 * must keep stream live until future resolved. (for pointer of stream)
	 */
	template <class T, std::enable_if_t<std::is_convertible_v<
		decltype(std::declval<T>().get()), InputStreamBase*>>* = nullptr>
	seastar::future<SharedString> readAll(const T& stream) {
		if (CPV_UNLIKELY(stream.get() == nullptr)) {
			return seastar::make_ready_future<SharedString>();
		}
		return readAll(*stream.get());
	}
}

