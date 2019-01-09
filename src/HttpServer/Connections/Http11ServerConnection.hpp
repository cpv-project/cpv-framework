#pragma once
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/HttpServer/HttpServerRequestHandler.hpp>

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
			seastar::lw_shared_ptr<const std::vector<std::unique_ptr<HttpServerRequestHandler>>> handlers,
			seastar::connected_socket&& fd);
	
	private:
		seastar::lw_shared_ptr<const HttpServerConfiguration> configuration;
		seastar::lw_shared_ptr<const std::vector<std::unique_ptr<HttpServerRequestHandler>>> handlers;
		SocketHolder socket_;
		Http11ServerConnectionState state_;
	};
}

