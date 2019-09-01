#pragma once
#include "./ApplicationState.hpp"
#include "./ModuleBase.hpp"

namespace cpv {
	/** Members of Application */
	class ApplicationData;

	/**
	 * Generic purpose modular application class
	 *
	 * This class manage a collection of modules, modules decide what this
	 * application will do and how this application do it.
	 *
	 * After created application, you can add some modules to it like:
	 * ```
	 * Application application;
	 * application.add<SomeModule>();
	 * application.add<OtherModule>();
	 * ```
	 * and then run the application like:
	 * ```
	 * return application.start().then([] {
	 *     // do other works, and wait until program exit (e.g. Ctrl+C)
	 * }).then([application] () mutable {
	 *     return application.stop();
	 * });
	 * ```
	 * or use the convenient function run_forever:
	 * ```
	 * // start and stop function will capture internal data,
	 * // so destruct application instance after run_forever ready is ok
	 * return application.run_forever();
	 * ```
	 *
	 * Application will call the handle function defined in each module
	 * for different states, for now there 3 groups of states:
	 * - initialize states (when application.start first called)
	 * - starting states (each time application.start called)
	 * - stopping states (each time application.stop called)
	 * for example, if there 3 modules a,b,c in the application,
	 * when application.start first called, the application will invoke
	 * handle function in this order:
	 * - a.handle(container, StartInitialize)
	 * - b.handle(container, StartInitialize)
	 * - c.handle(container, StartInitialize)
	 * - a.handle(container, RegisterBasicServices)
	 * - b.handle(container, RegisterBasicServices)
	 * - c.handle(container, RegisterBasicServices)
	 * - ...
	 * - a.handle(container, AfterInitialized)
	 * - b.handle(container, AfterInitialized)
	 * - c.handle(container, AfterInitialized)
	 * - a.handle(container, BeforeStart)
	 * - b.handle(container, BeforeStart)
	 * - c.handle(container, BeforeStart)
	 * - a.handle(container, Starting)
	 * - a.handle(container, Starting)
	 * - b.handle(container, Starting)
	 * - c.handle(container, AfterStarted)
	 * - b.handle(container, AfterStarted)
	 * - c.handle(container, AfterStarted)
	 * Please also check ApplicationState.hpp for the description of each state.
	 *
	 * Follow seastar framework's share nothing design, application will create
	 * module instances and invoke the handle functions on each cpu core
	 * separately, data shared between cpu cores should be read only,
	 * otherwise you will need seastar's smp::submit_to to perform the mutation.
	 *
	 * Modules communicate with each other by a common interface, a dependency
	 * injection container will pass to handle function as the first argument,
	 * modules can register services to the container and get services from it
	 * later, the container will shared by all modules on the same cpu core.
	 *
	 * Module can be added with a custom initialize function, the function
	 * is for configure module dynamically, and will call between state
	 * BeforeCallCustomInitializeFunctions and AfterCustomInitializeFunctionsCalled.
	 * For example:
	 * ```
	 * application.add<LoggingModule>([] (auto& module) {
	 *     module.setLogger(Logger::CreateConsole(LogLevel::Info));
	 * });
	 * ```
	 * Notice module class should defines no argument constructor, if you
	 * want to pass something to module, use custom initialize function.
	 *
	 * The order of module matters, and it's same as the order application.add
	 * called, some services may registered multiple times, the order of
	 * services (from container.getMany) is dependent on the order of module,
	 * and on which state module register the service, for example there several
	 * modules, they register services in different states like:
	 * Module A:
	 * - register inst-q at RegisterBasicServices
	 * - register inst-w at RegisterServices
	 * Module B:
	 * - register inst-e at RegisterHeadServices
	 * - register inst-r at RegisterServices
	 * - register inst-t at RegisterTailServices
	 * Module C:
	 * - register inst-y at RegisterBasicServices
	 * - register inst-u at RegisterServices
	 * the order of services will be q, y, e, w, r, u, t.
	 *
	 * For more examples, please see the example folder of this project.
	 * For now it's most likely used to build a server side web application.
	 */
	class Application {
	public:
		/** Add module to application */
		template <class Module>
		void add() {
			add<Module>(nullptr);
		}

		/** Add module to application with custom initialize function */
		template <class Module>
		void add(std::function<void(Module&)> initializeFunction) {
			if (initializeFunction == nullptr) {
				add([] { return std::make_unique<Module>(); }, nullptr);
			} else {
				add([] { return std::make_unique<Module>(); },
					[func=std::move(initializeFunction)] (ModuleBase* module) {
						func(*static_cast<Module*>(module));
					});
			}
		}

		/** Start application */
		seastar::future<> start();

		/** Stop application */
		seastar::future<> stop();

		/** Run application until program exit (e.g. Ctrl+C) */
		seastar::future<> run_forever();

		/** Constructor */
		Application();

	private:
		/** Add module to application (implementation) */
		void add(
			const std::function<std::unique_ptr<ModuleBase>()>& moduleFactory,
			const std::function<void(ModuleBase*)>& initializeFunction);

	private:
		seastar::shared_ptr<ApplicationData> data_;
	};
}

