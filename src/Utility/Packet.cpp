#include <cstring>
#include <CPVFramework/Utility/Packet.hpp>

namespace cpv {
	/** The storage of Packet::MultipleFragments */
	template <>
	thread_local ReusableStorageType<Packet::MultipleFragments>
		ReusableStorageInstance<Packet::MultipleFragments>;

	/** Get MultipleFragments, or convert to MultipleFragments if it's not */
	Packet::MultipleFragments& Packet::getOrConvertToMultiple() & {
		if (auto ptr = getIfMultiple()) {
			return *ptr;
		}
		auto multipleFragments = makeReusable<Packet::MultipleFragments>();
		if (auto ptr = getIfSingle()) {
			if (ptr->fragment.size != 0) {
				multipleFragments->fragments.emplace_back(ptr->fragment);
				multipleFragments->deleter = std::move(ptr->deleter);
			}
		}
		auto multiplePtr = multipleFragments.get();
		data_ = std::move(multipleFragments);
		return *multiplePtr;
	}

	/** Append string to packet */
	Packet& Packet::append(SharedString&& str) & {
		if (auto ptr = getIfMultiple()) {
			ptr->append(std::move(str));
		} else if (auto ptr = getIfSingle()) {
			if (ptr->fragment.size == 0) {
				// replace empty single fragment
				data_ = Packet::SingleFragment(std::move(str));
			} else {
				// convert single fragment to multiple fragments
				auto multipleFragments = makeReusable<Packet::MultipleFragments>();
				multipleFragments->fragments.emplace_back(ptr->fragment);
				multipleFragments->deleter = std::move(ptr->deleter);
				multipleFragments->append(std::move(str));
				data_ = std::move(multipleFragments);
			}
		} else {
			// replace valueless
			data_ = Packet::SingleFragment(std::move(str));
		}
		return *this;
	}

	/** Append other packet to this packet */
	Packet& Packet::append(Packet&& other) & {
		if (auto thisPtr = getIfMultiple()) {
			if (auto otherPtr = other.getIfMultiple()) {
				// append multiple to multiple
				auto& thisFragments = thisPtr->fragments;
				auto& otherFragments = otherPtr->fragments;
				thisFragments.reserve(thisFragments.size() + otherFragments.size());
				thisFragments.insert(thisFragments.end(),
					otherFragments.begin(), otherFragments.end());
				thisPtr->deleter.append(std::move(otherPtr->deleter));
				otherFragments.clear();
			} else if (auto otherPtr = other.getIfSingle()) {
				// append single to multiple
				auto& thisFragments = thisPtr->fragments;
				auto& otherFragment = otherPtr->fragment;
				thisFragments.emplace_back(otherFragment);
				thisPtr->deleter.append(std::move(otherPtr->deleter));
				otherFragment.size = 0;
			}
		} else if (auto thisPtr = getIfSingle()) {
			if (auto otherPtr = other.getIfMultiple()) {
				// append multiple to single
				otherPtr->fragments.insert(otherPtr->fragments.begin(), thisPtr->fragment);
				otherPtr->deleter.append(std::move(thisPtr->deleter));
				data_ = std::move(other.data_);
			} else if (auto otherPtr = other.getIfSingle()) {
				// append single to single
				if (thisPtr->fragment.size == 0) {
					// this is empty
					thisPtr->fragment = otherPtr->fragment;
					thisPtr->deleter = std::move(otherPtr->deleter);
					otherPtr->fragment.size = 0;
				} else if (otherPtr->fragment.size != 0) {
					// convert single fragment to multiple fragments
					auto thisFragments = makeReusable<Packet::MultipleFragments>();
					thisFragments->fragments.emplace_back(thisPtr->fragment);
					thisFragments->fragments.emplace_back(otherPtr->fragment);
					thisFragments->deleter = std::move(thisPtr->deleter);
					thisFragments->deleter.append(std::move(otherPtr->deleter));
					data_ = std::move(thisFragments);
					otherPtr->fragment.size = 0;
				}
			}
		} else {
			// replace valueless
			data_ = std::move(other.data_);
		}
		return *this;
	}

	/** Get total size in bytes for this packet, notice it's dynamically calculated  */
	std::size_t Packet::size() const {
		if (auto ptr = getIfMultiple()) {
			std::size_t size = 0;
			for (auto& f : ptr->fragments) {
				size += f.size;
			}
			return size;
		} else if (auto ptr = getIfSingle()) {
			return ptr->fragment.size;
		}
		return 0;
	}

	/** Get the numnber of segments, may return 0 if empty */
	std::size_t Packet::segments() const {
		if (auto ptr = getIfMultiple()) {
			return ptr->fragments.size();
		} else if (auto ptr = getIfSingle()) {
			return ptr->fragment.size ? 1 : 0;
		}
		return 0;
	}

	/** Get whether this packet is empty (size == 0) */
	bool Packet::empty() const {
		if (auto ptr = getIfMultiple()) {
			return ptr->fragments.empty();
		} else if (auto ptr = getIfSingle()) {
			return ptr->fragment.size == 0;
		}
		return true;
	}

	/** Concat all fragments and return as string */
	SharedString Packet::toString() const {
		if (auto ptr = getIfMultiple()) {
			SharedString str(size());
			char* buf = str.data();
			for (auto& f : ptr->fragments) {
				if (CPV_LIKELY(f.size > 0)) {
					std::memcpy(buf, f.base, f.size);
					buf += f.size;
				}
			}
			return str;
		} else if (auto ptr = getIfSingle()) {
			// use const_cast to increase refcount
			return SharedString(
				ptr->fragment.base,
				ptr->fragment.size,
				const_cast<seastar::deleter&>(ptr->deleter).share());
		}
		return SharedString();
	}

	/** Print packet fragments */
	std::ostream& operator<<(std::ostream& stream, const Packet& packet) {
		if (auto ptr = packet.getIfMultiple()) {
			for (auto& f : ptr->fragments) {
				stream << std::string_view(f.base, f.size);
			}
		} else if (auto ptr = packet.getIfSingle()) {
			stream << std::string_view(ptr->fragment.base, ptr->fragment.size);
		}
		return stream;
	}

	/** Write packet fragments to string builder */
	SharedStringBuilder& operator<<(SharedStringBuilder& builder, const Packet& packet) {
		if (auto ptr = packet.getIfMultiple()) {
			builder.reserve(builder.size() + packet.size());
			for (auto& f : ptr->fragments) {
				builder.append(std::string_view(f.base, f.size));
			}
		} else if (auto ptr = packet.getIfSingle()) {
			builder.append(std::string_view(ptr->fragment.base, ptr->fragment.size));
		}
		return builder;
	}
}

