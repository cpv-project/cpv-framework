#pragma once
#include <string_view>
#include <variant>
#include <vector>
#include <iostream>
#include <seastar/core/temporary_buffer.hh>
#include <seastar/net/packet.hh>
#include "./Reusable.hpp"

namespace cpv {
	/**
	 * Class used to hold fragments and build seastar::packet or seastar::temporary_buffer
	 *
	 * Why not use seastar::packet directly:
	 * - seastar::packet require dynamic allocation even for single fragment
	 * - seastar::packet can't append multiple fragments and have to call move ctor for each append
	 *
	 * To optimize, this class holds a variant of single fragment and multiple fragments,
	 * single fragment can release to seastar::temporary_buffer and multiple fragments can release
	 * to seastar::packet.
	 *
	 * Notice:
	 * It's possible that both getIfSingle and getIfMultiple return nullptr, if it has been moved,
	 * and it's ensure to be empty after moved.
	 */
	class Packet {
	public:
		/**
		 * Convert std::string_view to seastar::net::fragment
		 * To compatible with c socket api, const_cast is used, the bytes will not be modified anyway.
		 */
		static inline seastar::net::fragment toFragment(const std::string_view& str) {
			return { const_cast<char*>(str.data()), str.size() };
		}
		
		/** Single fragment with deleter, deleter can be empty for static data */
		struct SingleFragment {
			seastar::net::fragment fragment;
			seastar::deleter deleter;

			seastar::temporary_buffer<char> release() {
				seastar::temporary_buffer<char> buf(fragment.base, fragment.size, std::move(deleter));
				fragment.size = 0;
				return buf;
			}

			SingleFragment() : fragment(), deleter() { }
			SingleFragment(const seastar::net::fragment& fragmentVal) :
				fragment(fragmentVal), deleter() { }
			SingleFragment(const seastar::net::fragment& fragmentVal, seastar::deleter&& deleterVal) :
				fragment(fragmentVal), deleter(std::move(deleterVal)) { }
		};

		/** Multiple fragments with deleter, deleter can be chained, also this class is reusable */
		struct MultipleFragments {
			std::vector<seastar::net::fragment> fragments;
			seastar::deleter deleter;

			void freeResources() {
				fragments.clear();
				deleter = seastar::deleter();
			}

			static void reset() { }

			seastar::net::packet release() {
				// seastar::packet's ctor provides overload for std::vector but only takes moved one
				// which will release the internal storage and we have to allocate it again for next
				// we want to keep the internal storage, so iterator overload is used here
				seastar::net::packet p(fragments.begin(), fragments.end(), std::move(deleter));
				fragments.clear();
				return p;
			};

			MultipleFragments() : fragments(), deleter() { }
		};

	public:
		/** Get SingleFragment, return nullptr if it's MultipleFragments */
		SingleFragment* getIfSingle() & {
			return std::get_if<SingleFragment>(&data_);
		}

		/** Get SingleFragment, return nullptr if it's MultipleFragments */
		const SingleFragment* getIfSingle() const& {
			return std::get_if<SingleFragment>(&data_);
		}

		/** Get MultipleFragments, return nullptr if it's SingleFragment */
		MultipleFragments* getIfMultiple() & {
			if (auto ptr = std::get_if<Reusable<MultipleFragments>>(&data_)) {
				return ptr->get();
			}
			return nullptr;
		}

		/** Get MultipleFragments, return nullptr if it's SingleFragment */
		const MultipleFragments* getIfMultiple() const& {
			if (auto ptr = std::get_if<Reusable<MultipleFragments>>(&data_)) {
				return ptr->get();
			}
			return nullptr;
		}

		/** Append static string to packet */
		template <std::size_t Size>
		Packet& append(const char(*str)[Size]) {
			return append(std::string_view(str, Size));
		}

		/** Append static string to packet */
		Packet& append(const std::string_view& str) &;

		/** Append dynamic string and it's deleter to packet */
		Packet& append(const std::string_view& str, seastar::deleter&& deleter) &;

		/** Append temporary_buffer to packet */
		Packet& append(seastar::temporary_buffer<char>&& buf) &;

		/** Append other packet to this packet */
		Packet& append(Packet&& other) &;

		/** Get total size in bytes for this packet, notice it's dynamically calculated  */
		std::size_t size() const;

		/** Get whether this packet is empty (size == 0) */
		bool empty() const;

		/** Constructor */
		Packet() : data_(SingleFragment()) { }

		/** Append static string to packet */
		template <std::size_t Size>
		Packet(const char(*str)[Size]) :
			data_(SingleFragment(toFragment(std::string_view(str, Size)))) { }

		/** Constructor with static string */
		Packet(const std::string_view& str) :
			data_(SingleFragment(toFragment(str))) { }

		/** Constructor with dynamic string and it's deleter to packet */
		Packet(const std::string_view& str, seastar::deleter&& deleter) :
			data_(SingleFragment(toFragment(str), std::move(deleter))) { }

		/** Constructor with temporary_buffer */
		Packet(seastar::temporary_buffer<char>&& buf) :
			data_(SingleFragment({ buf.get_write(), buf.size() }, buf.release())) { }

		/** Constructor with capacity prepare for multiple fragments */
		Packet(std::size_t capacity) :
			data_([capacity] {
				auto data = makeReusable<MultipleFragments>();
				data->fragments.reserve(capacity);
				return data;
			}()) { }

	private:
		std::variant<SingleFragment, Reusable<MultipleFragments>> data_;
	};

	/** Print packet fragments */
	std::ostream& operator<<(std::ostream& stream, const Packet& packet);

	/** Increase free list size */
	template <>
	const constexpr std::size_t ReusableStorageCapacity<Packet::MultipleFragments> = 28232;
}

