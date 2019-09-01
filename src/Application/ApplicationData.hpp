#pragma once
#include <vector>
#include <memory>
#include <seastar/core/sharded.hh>
#include <CPVFramework/Application/ModuleBase.hpp>

namespace cpv {
	/** ApplicationData for per core */
	class ApplicationPerCoreData {
	public:
		/** For seastar::sharded */
		seastar::future<> stop();

		/** Constructor */
		ApplicationPerCoreData();

	public:
		Container container;
		std::vector<std::unique_ptr<ModuleBase>> modules;
	};

	/** Members of Application */
	class ApplicationData {
	public:
		/** Constructor */
		ApplicationData();

		/** Destructor */
		~ApplicationData();

	public:
		ApplicationState state;
		std::vector<std::pair<
			std::function<std::unique_ptr<ModuleBase>()>,
			std::function<void(ModuleBase*)>>> modulesFactories;
		std::unique_ptr<seastar::sharded<ApplicationPerCoreData>> shardData;
	};
}

