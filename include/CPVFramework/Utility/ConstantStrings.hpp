#pragma once
#include <array>
#include <string>

namespace cpv::constants {
	// server name
	extern const std::string CPVFramework;
	
	// common strings
	extern const std::string Space;
	extern const std::string ColonSpace;
	extern const std::string LF;
	extern const std::string CRLF;
	
	// numbers
	static const std::size_t MaxConstantInteger = 65535;
	extern const std::array<std::string, MaxConstantInteger+1> Integers;
}

