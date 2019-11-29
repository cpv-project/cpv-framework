#pragma once
#include <array>
#include <string>
#include <string_view>

namespace cpv::constants {
	// server name
	static const constexpr char CPVFramework[] = "cpv-framework";
	
	// common strings
	static const constexpr char Space[] = " ";
	static const constexpr char Tab[] = "\t";
	static const constexpr char Colon[] = ":";
	static const constexpr char SemiColon[] = ";";
	static const constexpr char Slash[] = "/";
	static const constexpr char Comma[] = ",";
	static const constexpr char QuestionMark[] = "?";
	static const constexpr char EqualsSign[] = "=";
	static const constexpr char Ampersand[] = "&";
	static const constexpr char Hyphen[] = "-";
	static const constexpr char SingleQuote[] = "'";
	static const constexpr char DoubleQuote[] = "\"";
	static const constexpr char RoundBacketStart[] = "(";
	static const constexpr char RoundBacketEnd[] = ")";
	static const constexpr char CurlyBacketStart[] = "{";
	static const constexpr char CurlyBacketEnd[] = "}";
	static const constexpr char SquareBacketStart[] = "[";
	static const constexpr char SquareBacketEnd[] = "]";
	static const constexpr char ColonSpace[] = ": ";
	static const constexpr char ColonSlashSlash[] = "://";
	static const constexpr char LF[] = "\n";
	static const constexpr char CRLF[] = "\r\n";
	static const constexpr char True[] = "true";
	static const constexpr char False[] = "false";
	static const constexpr char Null[] = "null";
	
	// numbers
	static const std::size_t MaxConstantInteger = 65535;
	extern const std::array<std::string, MaxConstantInteger+1> Integers;
}

