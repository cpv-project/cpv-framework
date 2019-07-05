#include <cstdlib>
#include <seastar/net/api.hh>
#include <seastar/net/inet_address.hh>
#include <CPVFramework/Utility/NetworkUtils.hpp>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Exceptions/NotImplementedException.hpp>

namespace cpv {
	/** Parse socket listen address */
	seastar::socket_address parseListenAddress(const std::string& address) {
		std::size_t index = address.find_first_of(':');
		if (CPV_UNLIKELY(index == std::string::npos)) {
			throw FormatException(CPV_CODEINFO, "no ':' in listen address:", address);
		}
		seastar::net::inet_address inetAddress;
		try {
			inetAddress = seastar::net::inet_address(
				index > 0 ? address.substr(0, index) : "0.0.0.0");
		} catch (const std::invalid_argument&) {
			throw FormatException(CPV_CODEINFO, "invalid listen ip address:", address);
		}
		if (CPV_UNLIKELY(inetAddress.in_family() != seastar::net::inet_address::family::INET)) {
			// seastar's socket_address only support ipv4 now so throw an exception for ipv6
			throw NotImplementedException(CPV_CODEINFO, "ipv6 address isn't supported yet");
		}
		char* endptr = nullptr;
		std::uint64_t port = std::strtoull(address.data() + index + 1, &endptr, 10);
		if (CPV_UNLIKELY(endptr != address.data() + address.size() || port < 1 || port > 0xffff)) {
			throw FormatException(CPV_CODEINFO, "invalid listen port:", address);
		}
		return seastar::socket_address(seastar::ipv4_addr(inetAddress, port));
	}
}

