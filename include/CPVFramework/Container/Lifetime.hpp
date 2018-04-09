#pragma once

namespace cpv {
	/** Lifetime of a service */
	enum class Lifetime {
		/** Create a new instance every times */
		Transient = 0,
		/** Create a new instance at first time and reuse it every times */
		Singleton = 1
	};
}

