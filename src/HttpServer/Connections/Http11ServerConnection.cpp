#include <seastar/core/sleep.hh>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Exceptions/LengthException.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Exceptions/NotImplementedException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include "Http11ServerConnection.hpp"

namespace cpv {
	namespace {
		static inline void flushMergedTemporaryBuffer(
			HttpRequest& request,
			seastar::temporary_buffer<char>& buffer,
			std::string_view& view) {
			if (CPV_UNLIKELY(buffer.size() > 0)) {
				auto bufView = request.addUnderlyingBuffer(std::move(buffer));
				view = bufView.substr(0, view.size());
			}
		}
		
		static inline std::string_view getHttpVersionString(const ::http_parser& parser) {
			if (CPV_LIKELY(parser.http_major == 1)) {
				if (CPV_LIKELY(parser.http_minor == 1)) {
					return constants::Http11;
				} else if (parser.http_minor == 0) {
					return constants::Http10;
				} else if (parser.http_minor == 2) {
					return constants::Http12;
				}
			}
			return std::string_view();
		}
	}
	
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
			// TODO: check state
			return self->socket_.in().read().then(
				[self] (seastar::temporary_buffer<char> buf) {
				// check bytes limitation of initial request data
				// no overflow check of receivedBytes because the buffer size should be small
				// if receivedBytes + buffer size cause overflow that mean the limitation is too large
				self->parserTemporaryData_.receivedBytes += buf.size();
				if (CPV_UNLIKELY(self->parserTemporaryData_.receivedBytes >
					self->sharedData_->configuration.getMaxInitialRequestBytes())) {
					// TODO: reply error response
					return seastar::make_exception_future<>(LengthException(
						CPV_CODEINFO, "http request length error:",
						"reached bytes limitation of initial request data"));
				}
				// check limitation of received packets, to avoid small packet attack
				self->parserTemporaryData_.receivedPackets += 1;
				if (CPV_UNLIKELY(self->parserTemporaryData_.receivedPackets >
					self->sharedData_->configuration.getMaxInitialRequestPackets())) {
					// TODO: reply error response
					return seastar::make_exception_future<>(LengthException(
						CPV_CODEINFO, "http request length error:",
						"reached packets limitation of initial request data"));
				}
				// hold buffer in request, so callbacks can make string views based on argument
				std::string_view bufView = self->request_.addUnderlyingBuffer(std::move(buf));
				// execute http parser
				std::size_t parsedSize = ::http_parser_execute(
					&self->parser_,
					&self->parserSettings_,
					bufView.data(),
					bufView.size());
				if (parsedSize != bufView.size()) {
					// TODO: reply error response
					return seastar::make_exception_future<>(FormatException(
						CPV_CODEINFO, "http request format error:",
						::http_errno_name(static_cast<enum ::http_errno>(self->parser_.http_errno)),
						::http_errno_description(static_cast<enum ::http_errno>(self->parser_.http_errno))));
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
		state_(Http11ServerConnectionState::Initial),
		request_(),
		response_(),
		parserSettings_(),
		parser_(),
		parserTemporaryData_() {
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
		parser_.data = this;
	}
	
	int Http11ServerConnection::onMessageBegin(::http_parser* parser) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		self->state_ = Http11ServerConnectionState::ReceiveRequestMessageBegin;
		return 0;
	}
	
	int Http11ServerConnection::onUrl(::http_parser* parser, const char* data, std::size_t size) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestMessageBegin) {
			// the first time received the url
			self->state_ = Http11ServerConnectionState::ReceiveRequestUrl;
			self->parserTemporaryData_.urlView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// url splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				self->parserTemporaryData_.urlMerged,
				self->parserTemporaryData_.urlView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::onHeaderField(::http_parser* parser, const char* data, std::size_t size) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// the first time received a new header field, flush last header field and value
			self->state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
			flushMergedTemporaryBuffer(
				self->request_,
				self->parserTemporaryData_.headerFieldMerged,
				self->parserTemporaryData_.headerFieldView);
			flushMergedTemporaryBuffer(
				self->request_,
				self->parserTemporaryData_.headerValueMerged,
				self->parserTemporaryData_.headerValueView);
			self->request_.setHeader(
				self->parserTemporaryData_.headerFieldView,
				self->parserTemporaryData_.headerValueView);
			self->parserTemporaryData_.headerFieldView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// the first time received the first header field, flush method, url and version
			self->state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
			flushMergedTemporaryBuffer(
				self->request_,
				self->parserTemporaryData_.urlMerged,
				self->parserTemporaryData_.urlView);
			self->request_.setMethod(::http_method_str(
				static_cast<enum ::http_method>(self->parser_.method)));
			self->request_.setUrl(self->parserTemporaryData_.urlView);
			self->request_.setVersion(getHttpVersionString(self->parser_));
			self->parserTemporaryData_.headerFieldView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
			// header field splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				self->parserTemporaryData_.headerFieldMerged,
				self->parserTemporaryData_.headerFieldView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::onHeaderValue(::http_parser* parser, const char* data, std::size_t size) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
			// the first time received a header value after header field
			self->state_ = Http11ServerConnectionState::ReceiveRequestHeaderValue;
			self->parserTemporaryData_.headerValueView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// header value splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				self->parserTemporaryData_.headerValueMerged,
				self->parserTemporaryData_.headerValueView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::onHeadersComplete(::http_parser* parser) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// all headers received, flush last header field and value
			self->state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
			flushMergedTemporaryBuffer(
				self->request_,
				self->parserTemporaryData_.headerFieldMerged,
				self->parserTemporaryData_.headerFieldView);
			flushMergedTemporaryBuffer(
				self->request_,
				self->parserTemporaryData_.headerValueMerged,
				self->parserTemporaryData_.headerValueView);
			self->request_.setHeader(
				self->parserTemporaryData_.headerFieldView,
				self->parserTemporaryData_.headerValueView);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// no headers but url, flush method, url and version
			self->state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
			flushMergedTemporaryBuffer(
				self->request_,
				self->parserTemporaryData_.urlMerged,
				self->parserTemporaryData_.urlView);
			self->request_.setMethod(::http_method_str(
				static_cast<enum ::http_method>(self->parser_.method)));
			self->request_.setUrl(self->parserTemporaryData_.urlView);
			self->request_.setVersion(getHttpVersionString(self->parser_));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::onBody(::http_parser* parser, const char* data, std::size_t size) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		self->state_ = Http11ServerConnectionState::ReceiveRequestBody;
		std::cout << "body: " << std::string(data, size) << std::endl;
		return 0;
	}
	
	int Http11ServerConnection::onMessageComplete(::http_parser* parser) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		self->state_ = Http11ServerConnectionState::ReceiveRequestMessageComplete;
		std::cout << "message complete" << std::endl;
		return 0;
	}
}

