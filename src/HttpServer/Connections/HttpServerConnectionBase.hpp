#pragma once

namespace cpv {
	/** Interface of http server connection */
	class HttpServerConnectionBase {
	public:
		/** Virtual destructor */
		virtual ~HttpServerConnectionBase() = default;
		
		/** Stop the connection immediately */
		virtual seastar::future<> stop() = 0;
	};
}

