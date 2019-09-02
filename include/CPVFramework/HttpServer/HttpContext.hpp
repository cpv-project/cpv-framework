#pragma once
#include <seastar/net/api.hh>
#include "../Container/Container.hpp"
#include "../Http/HttpRequest.hpp"
#include "../Http/HttpResponse.hpp"

namespace cpv {
	/** The context type for handling single http request on http server */
	class HttpContext {
	public:
		/** The request received from client */
		HttpRequest request;

		/** The response reply to client */
		HttpResponse response;

		/** The socket address of client */
		seastar::socket_address clientAddress;

		/** The container shared by all concurrent requests on the same core */
		const Container container;

		/** The storage used only in this request for storage persistent services */
		ServiceStorage serviceStorage;

		/** Constructor */
		HttpContext() :
			request(),
			response(),
			clientAddress(seastar::make_ipv4_address(0, 0)),
			container(),
			serviceStorage() { }

		/** Constructor */
		HttpContext(
			seastar::socket_address&& clientAddressVal,
			const Container& containerVal) :
			request(),
			response(),
			clientAddress(std::move(clientAddressVal)),
			container(containerVal),
			serviceStorage() { }
	};
}

