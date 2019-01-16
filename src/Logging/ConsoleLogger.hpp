#include <CPVFramework/Logging/Logger.hpp>

namespace cpv {
	/** A logger writes messages to console */
	class ConsoleLogger : public Logger {
	public:
		using Logger::Logger;

	protected:
		/** The implmentation of log, writes to console */
		void logImpl(LogLevel logLevel, const std::string& message) override;
	};
}

