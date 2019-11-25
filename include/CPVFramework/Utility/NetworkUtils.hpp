#pragma once
#include <string_view>
#include <seastar/net/api.hh>

namespace cpv {
	/**
	 * Parse socket listen address.
	 * Supported format:
	 * - "ip:port", e.g. "0.0.0.0:80"
	 * - ":port", e.g. ":80"
	 * Hostname is unsupported.
	 */
	seastar::socket_address parseListenAddress(std::string_view address);
}

