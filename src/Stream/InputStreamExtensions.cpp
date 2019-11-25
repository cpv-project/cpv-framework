#include <algorithm>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>

namespace cpv::extensions {
	namespace {
		// avoid out-of-memory attack and handle overflow
		static const constexpr std::size_t MaxReservedCapacity = 1048576;
	}

	/** Read all data from stream and append to given string builder */
	seastar::future<> readAll(InputStreamBase& stream, SharedStringBuilder& builder) {
		std::size_t sizeHint = stream.sizeHint().value_or(0);
		if (sizeHint > 0) {
			// reserve buffer if size hint of stream is available
			builder.reserve(std::min(builder.size() + sizeHint, MaxReservedCapacity));
		}
		return seastar::repeat([&stream, &builder] {
			return stream.read().then([&builder] (auto&& result) {
				builder.append(result.data);
				return result.isEnd ?
					seastar::stop_iteration::yes :
					seastar::stop_iteration::no;
			});
		});
	}

	/** Read all data from stream and return it as string */
	seastar::future<SharedString> readAll(InputStreamBase& stream) {
		// optimize for fast path
		auto f = stream.read();
		if (f.available()) {
			if (CPV_LIKELY(!f.failed())) {
				InputStreamReadResult result = f.get0();
				if (result.isEnd) {
					// fast path
					return seastar::make_ready_future<SharedString>(std::move(result.data));
				} else {
					f = seastar::make_ready_future<InputStreamReadResult>(std::move(result));
				}
			} else {
				return seastar::make_exception_future<SharedString>(f.get_exception());
			}
		}
		// enter normal path
		return f.then([&stream] (auto&& result) {
			if (result.isEnd) {
				// the first result is the last result
				return seastar::make_ready_future<SharedString>(std::move(result.data));
			}
			SharedStringBuilder builder;
			std::size_t sizeHint = stream.sizeHint().value_or(0);
			if (sizeHint > 0) {
				// reserve buffer if size hint of stream is available
				builder.reserve(std::min(sizeHint - result.data.size(), MaxReservedCapacity));
			}
			builder.append(result.data);
			return seastar::do_with(std::move(builder), [&stream] (auto& builder) {
				return seastar::repeat([&stream, &builder] {
					return stream.read().then([&builder] (auto&& result) {
						builder.append(result.data);
						return result.isEnd ?
							seastar::stop_iteration::yes :
							seastar::stop_iteration::no;
					});
				}).then([&builder] {
					return builder.build();
				});
			});
		});
	}
}

