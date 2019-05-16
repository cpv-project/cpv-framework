#pragma once

namespace cpv {
	/** Defines how to manage service lifetime */
	enum class ServiceLifetime {
		/** Create new instance every times */
		Transient = 0,
		/** For given container, only create instance once and reuse it in the future (a.k.a Singleton) */
		Presistent = 1,
		/** For given storage, only create instance once and reuse it in the future (a.k.a Scoped) */
		StoragePresistent = 2
	};
}

