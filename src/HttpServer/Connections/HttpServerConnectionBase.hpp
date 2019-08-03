#pragma once

namespace cpv {
	/** Interface of http server connection */
	class HttpServerConnectionBase {
	public:
		/** Virtual destructor */
		virtual ~HttpServerConnectionBase() = default;
		
		/** Stop the connection immediately */
		virtual seastar::future<> stop() = 0;
		
		/** Invoke when timeout is detected from HttpServer's timer */
		virtual void onTimeout() = 0;
		
		/** Invoke when some progress made from connection */
		void resetDetectTimeoutFlag() { detectTimeoutFlag_ = false; }
		
	private:
		friend class HttpServer;
		
		// used to detect connection timeout (apply for all operations)
		// false -> (after first tick) true -> (after second tick) invoke on_timeout
		// the connection should set this value to false after each time progress made
		bool detectTimeoutFlag_ = false;
	};
}

