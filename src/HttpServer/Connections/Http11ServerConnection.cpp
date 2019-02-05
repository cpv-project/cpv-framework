#include <seastar/core/sleep.hh>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Exceptions/NotImplementedException.hpp>
#include "Http11ServerConnection.hpp"

namespace cpv {
	/** Start receive requests and send responses */
	void Http11ServerConnection::start() {
		// check state
		if (CPV_UNLIKELY(state_ != Http11ServerConnectionState::Initial)) {
			throw LogicException(
				CPV_CODEINFO, "can't start http connection not at initial state");
		}
		// start receive requests
		auto self = shared_from_this();
		seastar::do_until(
			[self] { return self->state_ == Http11ServerConnectionState::Closing; },
			[self] {
			// TODO
			return self->socket_.in().read().then(
				[self] (seastar::temporary_buffer<char> buf) {
				std::size_t parsed = ::http_parser_execute(
					&self->parser_, &self->parserSettings_, buf.get(), buf.size());
				if (parsed != buf.size()) {
					return seastar::make_exception_future<>(FormatException(
						CPV_CODEINFO, "http request format error:",
						::http_errno_description(HTTP_PARSER_ERRNO(&self->parser_))));
				}
				return seastar::make_ready_future<>();
			});
		}).handle_exception([self] (std::exception_ptr ex) {
			self->sharedData_->logger->log(LogLevel::Info,
				"abort http connection from:", self->clientAddress_, "because of", ex);
		}).then([self] {
			// remove self from connections collection (it's weak_ptr)
			auto* connectionsPtr = self->sharedData_->connectionsWrapper.get();
			std::size_t connectionsCount = 0;
			if (connectionsPtr != nullptr) {
				connectionsPtr->value.erase(self);
				connectionsCount = connectionsPtr->value.size();
			}
			// log and update state to closed
			self->sharedData_->logger->log(LogLevel::Info,
				"close http connection from:", self->clientAddress_,
				"(count:", connectionsCount, ")");
			self->state_ = Http11ServerConnectionState::Closed;
		});
	}
	
	/** Stop the connection immediately */
	seastar::future<> Http11ServerConnection::stop() {
		// update state to closing
		state_ = Http11ServerConnectionState::Closing;
		// abort reader and writer
		if (socket_.isConnected()) {
			socket_.socket().shutdown_output();
			socket_.socket().shutdown_input();
		}
		// wait until connection closed
		// check state every seconds instead of allocate promise object,
		// because stop connection is a rare operation
		auto self = shared_from_this();
		return seastar::do_until(
			[self] { return self->state_ == Http11ServerConnectionState::Closed; },
			[] { return seastar::sleep(std::chrono::seconds(1)); });
	}
	
	/** Constructor */
	Http11ServerConnection::Http11ServerConnection(
		const seastar::lw_shared_ptr<HttpServerSharedData>& sharedData,
		seastar::connected_socket&& fd,
		seastar::socket_address&& addr) :
		sharedData_(sharedData),
		socket_(std::move(fd)),
		clientAddress_(std::move(addr)),
		parserSettings_(),
		parser_(),
		state_(Http11ServerConnectionState::Initial),
		request_(),
		response_() {
		// setup http parser
		::http_parser_settings_init(&parserSettings_);
		::http_parser_init(&parser_, HTTP_REQUEST);
		parserSettings_.on_message_begin = onMessageBegin;
		parserSettings_.on_url = onUrl;
		parserSettings_.on_header_field = onHeaderField;
		parserSettings_.on_header_value = onHeaderValue;
		parserSettings_.on_headers_complete = onHeadersComplete;
		parserSettings_.on_body = onBody;
		parserSettings_.on_message_complete = onMessageComplete;
		parserSettings_.on_chunk_header = onChunkHeader;
		parserSettings_.on_chunk_complete = onChunkComplete;
		parser_.data = this;
	}
	
	int Http11ServerConnection::onMessageBegin(::http_parser* parser) {
		auto* connection = reinterpret_cast<Http11ServerConnection*>(parser->data);
		connection->state_ = Http11ServerConnectionState::ReceiveRequestMessageBegin;
		std::cout << "message begin" << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onUrl(::http_parser* parser, const char* data, std::size_t size) {
		auto* connection = reinterpret_cast<Http11ServerConnection*>(parser->data);
		connection->state_ = Http11ServerConnectionState::ReceiveRequestUrl;
		std::cout << "url: " << std::string(data, size) << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onHeaderField(::http_parser* parser, const char* data, std::size_t size) {
		auto* connection = reinterpret_cast<Http11ServerConnection*>(parser->data);
		connection->state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
		std::cout << "header field: " << std::string(data, size) << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onHeaderValue(::http_parser* parser, const char* data, std::size_t size) {
		auto* connection = reinterpret_cast<Http11ServerConnection*>(parser->data);
		connection->state_ = Http11ServerConnectionState::ReceiveRequestHeaderValue;
		std::cout << "header value: " << std::string(data, size) << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onHeadersComplete(::http_parser* parser) {
		auto* connection = reinterpret_cast<Http11ServerConnection*>(parser->data);
		connection->state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
		std::cout << "headers complete" << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onBody(::http_parser* parser, const char* data, std::size_t size) {
		auto* connection = reinterpret_cast<Http11ServerConnection*>(parser->data);
		connection->state_ = Http11ServerConnectionState::ReceiveRequestBody;
		std::cout << "body: " << std::string(data, size) << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onMessageComplete(::http_parser* parser) {
		auto* connection = reinterpret_cast<Http11ServerConnection*>(parser->data);
		connection->state_ = Http11ServerConnectionState::ReceiveRequestMessageComplete;
		std::cout << "message complete" << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onChunkHeader(::http_parser* parser) {
		std::cout << "chunk header" << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onChunkComplete(::http_parser* parser) {
		std::cout << "chunk complete" << std::endl;
		return 0;
	}
}

