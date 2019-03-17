#pragma once

namespace cpv {
	/** Type of a service lifetime */
	enum class ServiceLifetime {
		/** Create new instance every times */
		Transient = 0,
		/**
		 * For given storage, only create one instance and reuse it every times.
		 * This type can use for both singleton and scoped service.
		 */
		Presistent = 1
	};
}

