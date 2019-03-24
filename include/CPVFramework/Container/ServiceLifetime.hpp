#pragma once

namespace cpv {
	/** Defines how to manage service lifetime */
	enum class ServiceLifetime {
		/** Create new instance every times */
		Transient = 0,
		/** For given container, only create one instance and reuse it every times */
		Presistent = 1,
		/** For given storage, only create one instance and reuse it every times */
		StoragePresistent = 2
	};
}

