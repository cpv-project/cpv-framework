#pragma once
#include <tuple>
#include <utility>

namespace cpv {
	/** Combine two hash value, same as boost implementation */
	static inline std::size_t hashCombine(std::size_t a, std::size_t b) {
		// see this link for the magic number:
		// http://stackoverflow.com/questions/4948780
		return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
	}

	/** Provide a generic hash function for tuple types (include pair) */
	template <class Tuple,
		template <class T> class Hash = std::hash,
		std::enable_if_t<(std::tuple_size_v<Tuple> >= 0), int> = 0>
	class hash {
	private:
		template <std::size_t Index>
		static std::size_t calculate(const Tuple& value) {
			auto hash = Hash<std::tuple_element_t<Index, Tuple>>()(std::get<Index>(value));
			if constexpr (Index == 0) {
				return hash;
			} else {
				return hashCombine(hash, calculate<Index - 1>(value));
			}
		}

	public:
		std::size_t operator()(const Tuple& value) const {
			if constexpr (std::tuple_size_v<Tuple> == 0) {
				return 0;
			} else {
				return calculate<std::tuple_size_v<Tuple> - 1>(value);
			}
		}
	};
}

