#pragma once
#include "../Module/Module.hpp"
#include "../Http/Server.hpp"

namespace cpv {
	/**
	 * The base class of applications.
	 * Provide your own application class and implement registerModules.
	 * Please use ApplicationControl to create and manage application instances on multiple cores.
	 */
	class Application {
	public:
		/** Register modules, need to implement in your own application class */
		virtual seastar::future<> registerModules() = 0;

		/** Register single module by it's type, usually called from registerModules */
		template <class TModule>
		void registerModule() {
			modules_.emplace_back(seastar::make_shared<TModule>(configuration_));
		}

		/** Start application */
		seastar::future<> start();

		/** Stop application */
		seastar::future<> stop();

		/** Constructor */
		Application();
		explicit Application(const std::string& configurationJson);

		/** Virtual destructor */
		virtual ~Application() = default;

	protected:
		seastar::shared_ptr<const Json> configuration_;
		seastar::shared_ptr<Container> container_;
		seastar::shared_ptr<httpd::http_server> server_;
		std::vector<seastar::shared_ptr<Module>> modules_;
		bool started_;
	};
}

