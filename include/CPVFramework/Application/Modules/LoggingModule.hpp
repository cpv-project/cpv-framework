#pragma once
#include "../../Logging/Logger.hpp"
#include "../ModuleBase.hpp"

namespace cpv {
	/**
	 * Module to provide logger instance
	 *
	 * By default, it uses console logger with Notice level,
	 * you can set custom logger by using custom initialize function:
	 * ```
	 * application.add<LoggingModule>(auto& module) {
	 *     module.setLogger(Logger::createConsole(LogLevel::Info));
	 * });
	 * ```
	 * and get logger instance from the container:
	 * ```
	 * auto logger = container.get<seastar::shared_ptr<Logger>>();
	 * ```
	 *
	 * The lifetime of logger instance is persistent.
	 */
	class LoggingModule : public ModuleBase {
	public:
		/** Set custom logger */
		void setLogger(const seastar::shared_ptr<Logger>& logger);

		/** Do some work for given application state */
		seastar::future<> handle(Container& container, ApplicationState state) override;

		/** Constructor */
		LoggingModule();

	private:
		seastar::shared_ptr<Logger> logger_;
	};
}

