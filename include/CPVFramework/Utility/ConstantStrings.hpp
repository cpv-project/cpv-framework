#pragma once
#include <array>
#include <string>
#include <string_view>

namespace cpv::constants {
	// server name
	static const constexpr std::string_view CPVFramework("cpv-framework");
	
	// common strings
	static const constexpr std::string_view Space(" ");
	static const constexpr std::string_view Tab("\t");
	static const constexpr std::string_view Colon(":");
	static const constexpr std::string_view SemiColon(";");
	static const constexpr std::string_view Slash("/");
	static const constexpr std::string_view Comma(",");
	static const constexpr std::string_view QuestionMark("?");
	static const constexpr std::string_view EqualsSign("=");
	static const constexpr std::string_view Ampersand("&");
	static const constexpr std::string_view ColonSpace(": ");
	static const constexpr std::string_view ColonSlashSlash("://");
	static const constexpr std::string_view LF("\n");
	static const constexpr std::string_view CRLF("\r\n");
	
	// numbers
	static const std::size_t MaxConstantInteger = 65535;
	extern const std::array<std::string, MaxConstantInteger+1> Integers;
}

