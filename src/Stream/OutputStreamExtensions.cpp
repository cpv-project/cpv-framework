#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>

namespace cpv::extensions {
	/** Write string (rvalue) to stream */
	seastar::future<> writeAll(OutputStreamBase& stream, std::string&& str) {
		return seastar::do_with(std::move(str), [&stream] (auto& str) {
			return stream.write(str.data(), str.size());
		});
	}
}

