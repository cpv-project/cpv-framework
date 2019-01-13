#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <CPVFramework/Exceptions/OverflowException.hpp>

namespace cpv::extensions {
	namespace {
		static const std::size_t ReadBufferSize = 4096;
	}
	
	/** Read all data from stream and append to given string */
	seastar::future<> readAll(InputStreamBase& stream, std::string& str) {
		return seastar::repeat([&stream, &str] {
			std::size_t oldSize = str.size();
			if (CPV_UNLIKELY(str.max_size() - ReadBufferSize < oldSize)) {
				return seastar::make_exception_future<seastar::stop_iteration>(
					OverflowException(CPV_CODEINFO, "string size overflow"));
			}
			str.resize(oldSize + ReadBufferSize);
			return stream.read(str.data() + oldSize, ReadBufferSize)
				.then([&str] (auto&& result) {
				str.resize(str.size() - ReadBufferSize + result.size);
				return result.eof ?
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
}

