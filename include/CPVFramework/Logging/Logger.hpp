#pragma once
#include <core/shared_ptr.hh>
#include "../Utility/EnumUtils.hpp"
#include "../Utility/StringUtils.hpp"

namespace cpv {
	/** For logger */
	enum class LogLevel {
		Emergency = 0,
		Alert = 1,
		Critical = 2,
		Error = 3,
		Warning = 4,
		Notice = 5,
		Info = 6,
		Debug = 7
	};

	template <>
	struct EnumDescriptions<LogLevel> {
		static const std::vector<std::pair<LogLevel, const char*>>& get();
	};

	/** Interface use to log messages */
	class Logger {
	public:
		/** Get the log level */
		LogLevel getLogLevel() const { return logLevel_; }

		/** Set the log level */
		void setLogLevel(LogLevel logLevel) { logLevel_ = logLevel; }

		/** Check whether this log level is enabled */
		bool isEnabled(LogLevel logLevel) const { return logLevel <= logLevel_; }

		/** Log message with specified log level, it may do nothing if the level is not enabled */
		template <class... Args>
		void log(LogLevel logLevel, const Args&... args) {
			if (isEnabled(logLevel)) {
				logImpl(logLevel, joinString("",
					"<CPV:", logLevel, "> ", joinString(" ", args...), '\n'));
			}
		}

		/** Constructor */
		explicit Logger(LogLevel logLevel);

		/** Virtual destructor */
		virtual ~Logger() = default;

		/** Create a console logger */
		static seastar::shared_ptr<Logger> createConsole(LogLevel logLevel);

		/** Create a noop logger */
		static seastar::shared_ptr<Logger> createNoop();

	protected:
		/** The implmentation of log, may write to console or database */
		virtual void logImpl(LogLevel logLevel, const std::string& message) = 0;

	private:
		LogLevel logLevel_;
	};
}

