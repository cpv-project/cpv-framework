#include <seastar/core/future-util.hh>
#include "./ApplicationData.hpp"

namespace cpv {
	/** For seastar::sharded */
	seastar::future<> ApplicationPerCoreData::stop() {
		return seastar::make_ready_future<>();
	}

	/** Constructor */
	ApplicationPerCoreData::ApplicationPerCoreData() :
		container(),
		modules() { }

	/** Constructor */
	ApplicationData::ApplicationData() :
		state(ApplicationState::StartInitialize),
		modulesFactories(),
		shardData(std::make_unique<seastar::sharded<ApplicationPerCoreData>>()) { }

	/** Destructor */
	ApplicationData::~ApplicationData() {
		if (!shardData) {
			return;
		}
		// destruct modules on all cpu cores asynchronously
		(void)seastar::do_with(std::move(shardData), [] (auto& shardData) {
			return shardData->stop();
		});
	}
}

