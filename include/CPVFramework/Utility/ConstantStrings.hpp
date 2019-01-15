#pragma once
#include <array>
#include <string>

namespace cpv::constants {
	// numbers
	static const std::size_t MaxConstantInteger = 65535;
	extern const std::array<std::string, MaxConstantInteger+1> Integers;
}

