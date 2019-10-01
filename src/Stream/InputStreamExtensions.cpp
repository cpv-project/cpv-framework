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
}

