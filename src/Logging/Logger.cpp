#include <seastar/core/reactor.hh>
#include <CPVFramework/Logging/Logger.hpp>
#include "./ConsoleLogger.hpp"
#include "./NoopLogger.hpp"

namespace cpv {
	const std::vector<std::pair<LogLevel, const char*>>&
		EnumDescriptions<LogLevel>::get() {
		static std::vector<std::pair<LogLevel, const char*>> staticNames({
			{ LogLevel::Emergency, "Emergency" },
			{ LogLevel::Alert, "Alert" },
			{ LogLevel::Critical, "Critical" },
			{ LogLevel::Error, "Error" },
			{ LogLevel::Warning, "Warning" },
			{ LogLevel::Notice, "Notice" },
			{ LogLevel::Info, "Info" },
			{ LogLevel::Debug, "Debug" },
		});
		return staticNames;
	}

	/** Constructor */
	Logger::Logger(LogLevel logLevel) :
		logLevel_(logLevel) { }

	/** Create a console logger */
	seastar::shared_ptr<Logger> Logger::createConsole(LogLevel logLevel) {
		return seastar::make_shared<ConsoleLogger>(logLevel);
	}

	/** Create a noop logger */
	seastar::shared_ptr<Logger> Logger::createNoop() {
		return seastar::make_shared<NoopLogger>();
	}

	/** Get thread id for logging */
	std::size_t Logger::getThreadId() {
		return seastar::engine().cpu_id();
	}
}

