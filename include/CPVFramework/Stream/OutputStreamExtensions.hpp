#pragma once
#include "../Exceptions/LogicException.hpp"
#include "../Utility/SharedString.hpp"
#include "./OutputStreamBase.hpp"

namespace cpv::extensions {
	/** Write string to stream, must keep stream alive until future resolved */
	static inline seastar::future<> writeAll(
		OutputStreamBase& stream, SharedString&& str) {
		return stream.write(Packet(std::move(str)));
	}

	/** Write packet to stream, must keep stream alive until future resolved */
	static inline seastar::future<> writeAll(
		OutputStreamBase& stream, Packet&& packet) {
		return stream.write(std::move(packet));
	}

	/** Write data to stream, must keep stream alive until future resolved */
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

