#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <CPVFramework/Exceptions/OverflowException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>

namespace cpv::extensions {
	namespace {
		static const std::size_t ReadBufferSize = 4096;
	}

	/** Read all data from stream and append to given string */
	seastar::future<> readAll(InputStreamBase& stream, std::string& str) {
		// pre allocate string if size hint of stream is available
		std::size_t sizeHint = stream.sizeHint().value_or(0);
		if (sizeHint > 0) {
			str.reserve(str.size() + sizeHint);
		}
		return seastar::repeat([&stream, &str] {
			return stream.read().then([&str] (auto&& result) {
				str.append(result.view());
				return result.isEnd ?
					seastar::stop_iteration::yes :
					seastar::stop_iteration::no;
			});
		});
	}

	/** Read all data from stream and return it as string */
	seastar::future<std::string> readAll(InputStreamBase& stream) {
		return seastar::do_with(std::string(), [&stream] (auto& str) {
			return readAll(stream, str).then([&str] {
				return std::move(str);
			});
		});
	}

	/** Read all data from stream and return it as buffer, must keep stream live until future resolved */
	seastar::future<seastar::temporary_buffer<char>> readAllAsBuffer(InputStreamBase& stream) {
		// optimize for fast path
		auto f = stream.read();
		seastar::temporary_buffer<char> buf;
		if (f.available()) {
			if (CPV_LIKELY(!f.failed())) {
				InputStreamReadResult result = f.get0();
				if (result.isEnd) {
					// fast path
					return seastar::make_ready_future<
						seastar::temporary_buffer<char>>(std::move(result.data));
				} else {
					buf = std::move(result.data);
				}
			} else {
				return seastar::make_exception_future<
					seastar::temporary_buffer<char>>(f.get_exception());
			}
		}
		// pre allocate buffer if size of stream is available
		std::string_view existsContent;
		std::size_t sizeHint = stream.sizeHint().value_or(0);
		if (sizeHint > 0) {
			seastar::temporary_buffer<char> newBuf(sizeHint);
			mergeContent(newBuf, existsContent, std::string_view(buf.get(), buf.size()));
			buf = std::move(newBuf);
		} else {
			existsContent = std::string_view(buf.get(), buf.size());
		}
		// append following parts to buffer
		return seastar::do_with(
			std::move(buf), existsContent,
			[&stream] (auto& buf, auto& existsContent) {
			return seastar::repeat([&stream, &buf, &existsContent] {
				return stream.read().then([&buf, &existsContent] (auto&& result) {
					mergeContent(buf, existsContent, result.view());
					return result.isEnd ?
						seastar::stop_iteration::yes :
						seastar::stop_iteration::no;
				});
			}).then([&buf, &existsContent] {
				buf.trim(existsContent.size());
				return std::move(buf);
			});
		});
	}
}

