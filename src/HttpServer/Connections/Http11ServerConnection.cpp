#include <CPVFramework/Exceptions/NotImplementedException.hpp>
#include "Http11ServerConnection.hpp"

namespace cpv {
	/** Start receive requests and send responses */
	void Http11ServerConnection::start() {
		// TODO
	}
	
	/** Stop the connection immediately */
	seastar::future<> Http11ServerConnection::stop() {
		// TODO
		return seastar::make_exception_future<>(
			NotImplementedException(CPV_CODEINFO));
	}
	
	/** Constructor */
	Http11ServerConnection::Http11ServerConnection(
		const seastar::lw_shared_ptr<HttpServerSharedData>& sharedData,
		seastar::connected_socket&& fd) :
		sharedData_(sharedData),
		socket_(std::move(fd)),
		state_(Http11ServerConnectionState::Initial),
		request_(),
		response_() { }
}

