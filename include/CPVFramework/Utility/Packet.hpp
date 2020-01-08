#pragma once
#include <cassert>
#include <variant>
#include <vector>
#include <iostream>
#include <seastar/net/api.hh>
#include <seastar/net/packet.hh>
#include "./Reusable.hpp"
#include "./SharedString.hpp"
#include "./SharedStringBuilder.hpp"

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
	 * For more efficient packet building, you can use getOrConvertToMultiple and append
	 * functions of MultipleFragments, so most operations are minimal and inlined.
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
		static inline seastar::net::fragment toFragment(std::string_view str) {
			return { const_cast<char*>(str.data()), str.size() };
		}
		
		/** Single fragment with deleter, deleter can be empty for static data */
		struct SingleFragment {
			seastar::net::fragment fragment;
			seastar::deleter deleter;

			/** Move content of fragment to temporary_buffer */
			seastar::temporary_buffer<char> release() {
				seastar::temporary_buffer<char> buf(fragment.base, fragment.size, std::move(deleter));
				fragment.size = 0;
				return buf;
			}

			SingleFragment() : fragment(), deleter() { }
			explicit SingleFragment(SharedString&& str) :
				fragment({ str.data(), str.size() }), deleter(str.release()) { }

			SingleFragment(const SingleFragment&) = delete;
			SingleFragment(SingleFragment&& other) :
				fragment(other.fragment), deleter(std::move(other.deleter)) {
				other.fragment.size = 0;
			}
			SingleFragment& operator=(const SingleFragment&) = delete;
			SingleFragment& operator=(SingleFragment&& other) {
				fragment = other.fragment;
				deleter = std::move(other.deleter);
				other.fragment.size = 0;
				return *this;
			}
		};

		/** Multiple fragments with deleter, deleter can be chained, also this class is reusable */
		struct MultipleFragments {
			std::vector<seastar::net::fragment> fragments;
			seastar::deleter deleter;

			/** For Reusable */
			void freeResources() {
				fragments.clear();
				deleter = seastar::deleter();
			}

			/** For Reusable */
			static void reset() { }

			/** Append string to fragments */
			CPV_INLINE void append(SharedString&& str) {
				fragments.emplace_back(seastar::net::fragment({ str.data(), str.size() }));
				deleter.append(str.release());
			}

			/** Append static string to fragments */
			template <std::size_t Size>
			CPV_INLINE void append(const char(&str)[Size]) {
				fragments.emplace_back(toFragment({ str, Size - 1 }));
				static_assert(Size >= 1, "static string should contains tailing zero");
			}

			/** Reserve addition capacity of fragments */
			CPV_INLINE void reserveAddition(std::size_t additionSize) {
				fragments.reserve(fragments.size() + additionSize);
			}

			/** Move content of fragments to packet */
			CPV_INLINE seastar::net::packet release() {
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

		/** Get MultipleFragments, or convert to MultipleFragments if it's not */
		MultipleFragments& getOrConvertToMultiple() &;

		/** Append string to packet */
		Packet& append(SharedString&& str) &;

		/** Append other packet to this packet */
		Packet& append(Packet&& other) &;

		/** Get total size in bytes for this packet, notice it's dynamically calculated  */
		std::size_t size() const;

		/** Get the numnber of segments, may return 0 if empty */
		std::size_t segments() const;

		/** Get whether this packet is empty (size == 0) */
		bool empty() const;

		/** Concat all fragments and return as string */
		SharedString toString() const;

		/** Constructor */
		Packet() : data_(SingleFragment()) { }

		/** Construct with string */
		explicit Packet(SharedString&& str) :
			data_(SingleFragment(std::move(str))) { }

		/** Constructor with capacity prepare for multiple fragments */
		explicit Packet(std::size_t capacity) :
			data_([capacity] {
				auto data = makeReusable<MultipleFragments>();
				data->fragments.reserve(capacity);
				return data;
			}()) { }

	private:
		std::variant<SingleFragment, Reusable<MultipleFragments>> data_;
	};

	/** Increase free list size */
	template <>
	const constexpr std::size_t ReusableStorageCapacity<Packet::MultipleFragments> = 28232;

	/** Print packet fragments */
	std::ostream& operator<<(std::ostream& stream, const Packet& packet);

	/** Write packet fragments to string builder */
	SharedStringBuilder& operator<<(SharedStringBuilder& builder, const Packet& packet);

	/** Write packet to seastar::data_sink */
	static inline seastar::future<> operator<<(seastar::data_sink& out, Packet&& packet) {
		if (auto ptr = packet.getIfSingle()) {
			return out.put(ptr->release()); // put seastar::temporary_buffer
		} else if (auto ptr = packet.getIfMultiple()) {
			return out.put(ptr->release()); // put seastar::net::packet
		} else {
			return seastar::make_ready_future<>();
		}
	}
}

