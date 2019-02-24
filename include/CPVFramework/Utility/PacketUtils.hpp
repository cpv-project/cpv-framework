#pragma once
#include <string_view>
#include <seastar/core/temporary_buffer.hh>
#include <seastar/net/packet.hh>

namespace cpv {
	// Use functions in this file is much simpler and a little faster
	// (no empty check and no initial allocate) than seastar::scattered_message
	
	/** Append static fragment to packet */
	static inline seastar::net::packet& operator<<(
		seastar::net::packet& packet, const std::string_view& data) {
		// fragment::data is read only, but doesn't have const qualifier
		packet = seastar::net::packet(
			std::move(packet),
			seastar::net::fragment{ const_cast<char*>(data.data()), data.size() },
			seastar::deleter());
		return packet;
	}
	
	/** Append static fragment to packet */
	template <std::size_t Size>
	static inline seastar::net::packet& operator<<(
		seastar::net::packet& packet, const char(*data)[Size]) {
		static_assert(Size > 0, "size of c string should not be 0");
		// fragment::data is read only, but doesn't have const qualifier
		packet = seastar::net::packet(
			std::move(packet),
			seastar::net::fragment{ const_cast<char*>(data), Size-1 },
			seastar::deleter());
		return packet;
	}
	
	/** Append temporary buffer to packet */
	static inline seastar::net::packet& operator<<(
		seastar::net::packet& packet, seastar::temporary_buffer<char>&& buf) {
		packet = seastar::net::packet(std::move(packet), std::move(buf));
		return packet;
	}
}

