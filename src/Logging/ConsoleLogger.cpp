#include <iostream>
#include "./ConsoleLogger.hpp"

namespace cpv {
	/** The implmentation of log, writes to console */
	void ConsoleLogger::logImpl(LogLevel logLevel, const std::string& message) {
		if (logLevel <= LogLevel::Warning) {
			std::cerr << message;
		} else {
			std::cout << message;
		}
	}
}

