#include <iostream>
#include "./ConsoleLogger.hpp"

namespace cpv {
	namespace {
		/** Colorize log output */
		static const char* getColorCodePrefix(LogLevel logLevel) {
			if (logLevel <= LogLevel::Error) {
				return "\e[31m"; // red
			} else if (logLevel <= LogLevel::Warning) {
				return "\e[33m"; // yellow
			} else if (logLevel<= LogLevel::Notice) {
				return "\e[32m"; // green
			} else if (logLevel >= LogLevel::Debug) {
				return "\e[90m"; // gray
			} else {
				return "";
			}
		}
		
		/** Reset default color */
		static const char* getColorCodeSuffix() {
			return "\e[0m";
		}
	}
	
	/** The implmentation of log, writes to console */
	void ConsoleLogger::logImpl(LogLevel logLevel, const std::string& message) {
		// build a single string first to avoid thread race
		if (logLevel <= LogLevel::Warning) {
			std::cerr << (getColorCodePrefix(logLevel) + message + getColorCodeSuffix());
		} else {
			std::cout << (getColorCodePrefix(logLevel) + message + getColorCodeSuffix());
		}
	}
}

