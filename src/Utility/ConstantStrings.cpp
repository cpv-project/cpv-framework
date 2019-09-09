#include <CPVFramework/Utility/ConstantStrings.hpp>

namespace cpv::constants {
	// numbers
	const std::array<std::string, MaxConstantInteger+1> Integers(([] {
		std::array<std::string, MaxConstantInteger+1> arr;
		for (std::size_t i = 0; i < arr.size(); ++i) {
			arr[i] = std::to_string(i);
		}
		return arr;
	})());
}

