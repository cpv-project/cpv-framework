#pragma once
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp>

namespace cpv {
	/** The state of a http 1.0/1.1 connection */
	enum class Http11ServerConnectionState {
		
	};
	
	/** Connection accepted from http client uses http 1.0/1.1 protocol */
	class Http11ServerConnection {
	public:
		/** Constructor */
		Http11ServerConnection(
			seastar::lw_shared_ptr<const HttpServerConfiguration> configuration,
			seastar::lw_shared_ptr<const std::vector<std::unique_ptr<HttpServerRequestHandlerBase>>> handlers,
			seastar::connected_socket&& fd);
	
	private:
		seastar::lw_shared_ptr<const HttpServerConfiguration> configuration_;
		seastar::lw_shared_ptr<const std::vector<std::unique_ptr<HttpServerRequestHandlerBase>>> handlers_;
		SocketHolder socket_;
		Http11ServerConnectionState state_;
		HttpRequest request_;
		HttpResponse response_;
	};
}

