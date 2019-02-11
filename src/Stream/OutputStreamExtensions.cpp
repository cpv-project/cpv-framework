#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>

namespace cpv::extensions {
	/** Write string (rvalue) to stream */
	seastar::future<> writeAll(OutputStreamBase& stream, std::string&& str) {
		return seastar::do_with(std::move(str), [&stream] (auto& str) {
			return stream.write(seastar::net::packet::from_static_data(str.data(), str.size()));
		});
	}
	
	/** Write string view to stream */
	seastar::future<> writeAll(OutputStreamBase& stream, const std::string_view& str) {
		return stream.write(seastar::net::packet::from_static_data(str.data(), str.size()));
	}
}

