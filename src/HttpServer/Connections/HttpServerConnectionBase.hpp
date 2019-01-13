#pragma once

namespace cpv {
	/** Interface of http server connection */
	class HttpServerConnectionBase {
	public:
		/** Virtual destructor */
		virtual ~HttpServerConnectionBase() = default;
		
		/** Close this connection */
		virtual seastar::future<> close() = 0;
	};
}

