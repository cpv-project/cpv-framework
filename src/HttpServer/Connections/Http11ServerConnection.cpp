#include <seastar/core/sleep.hh>
#include <seastar/net/packet.hh>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Exceptions/LengthException.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Utility/PacketUtils.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include "Http11ServerConnection.hpp"

namespace cpv {
	namespace {
		/** Static response strings */
		namespace {
			static const std::string ReachedBytesLimitationOfInitialRequestData(
				"HTTP/1.0 400 Bad Request\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 58\r\n"
				"Connection: close\r\n\r\n"
				"Error: reached bytes limitation of initial request data.\r\n");
			
			static const std::string ReachedPacketsLimitationOfInitialRequestData(
				"HTTP/1.0 400 Bad Request\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 60\r\n"
				"Connection: close\r\n\r\n"
				"Error: reached packets limitation of initial request data.\r\n");
			
			static const std::string InvalidHttpRequestFormat(
				"HTTP/1.0 400 Bad Request\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 37\r\n"
				"Connection: close\r\n\r\n"
				"Error: invalid http request format.\r\n");
			
			static const std::string InvalidStateAfterReceivedSingleRequest(
				"HTTP/1.0 500 Internal Server Error\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 53\r\n"
				"Connection: close\r\n\r\n"
				"Error: invalid state after received single request.\r\n");
		}
		
		/** Reply static response string and flush the output stream */
		static seastar::future<> replyStaticResponse(SocketHolder& s, const std::string_view& str) {
			return s.out().write(seastar::net::packet::from_static_data(str.data(), str.size()))
				.then([&s] { s.out().flush(); });
		}
		
		/** Move temporary buffer to request and update string view if necessary */
		static inline void flushMergedTemporaryBuffer(
			HttpRequest& request,
			seastar::temporary_buffer<char>& buffer) {
			if (CPV_UNLIKELY(buffer.size() > 0)) {
				request.addUnderlyingBuffer(std::move(buffer));
			}
		}
		
		/** Get http version string from parser, return empty for version not supported */
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
	
	/** Enum descriptions of Http11ServerConnectionState */
	const std::vector<std::pair<Http11ServerConnectionState, const char*>>&
		EnumDescriptions<Http11ServerConnectionState>::get() {
		static std::vector<std::pair<Http11ServerConnectionState, const char*>> staticNames({
			{ Http11ServerConnectionState::Initial, "Initial" },
			{ Http11ServerConnectionState::Started, "Started" },
			{ Http11ServerConnectionState::ReceiveRequestInitial, "ReceiveRequestInitial" },
			{ Http11ServerConnectionState::ReceiveRequestMessageBegin, "ReceiveRequestMessageBegin" },
			{ Http11ServerConnectionState::ReceiveRequestUrl, "ReceiveRequestUrl" },
			{ Http11ServerConnectionState::ReceiveRequestHeaderField, "ReceiveRequestHeaderField" },
			{ Http11ServerConnectionState::ReceiveRequestHeaderValue, "ReceiveRequestHeaderValue" },
			{ Http11ServerConnectionState::ReceiveRequestHeadersComplete, "ReceiveRequestHeadersComplete" },
			{ Http11ServerConnectionState::ReceiveRequestBody, "ReceiveRequestBody" },
			{ Http11ServerConnectionState::ReceiveRequestMessageComplete, "ReceiveRequestMessageComplete" },
			{ Http11ServerConnectionState::ReplyResponse, "ReplyResponse" },
			{ Http11ServerConnectionState::Closing, "Closing" },
			{ Http11ServerConnectionState::Closed, "Closed" },
		});
		return staticNames;
	}
	
	/** Start receive requests and send responses */
	void Http11ServerConnection::start() {
		// check state
		if (CPV_UNLIKELY(state_ != Http11ServerConnectionState::Initial)) {
			throw LogicException(
				CPV_CODEINFO, "can't start http connection not at initial state");
		}
		state_ = Http11ServerConnectionState::Started;
		// handle requests
		auto self = shared_from_this();
		seastar::do_until(
			[self] { return self->state_ == Http11ServerConnectionState::Closing; },
			[self] {
			// handle single request
			self->state_ = Http11ServerConnectionState::ReceiveRequestInitial;
			if (self->temporaryData_.messageCompleted) {
				// reset members if it's not the first request
				self->request_ = {};
				self->response_ = {};
				self->temporaryData_ = {};
			}
			return self->receiveSingleRequest().then([self] {
				return self->replySingleResponse();
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
		temporaryData_(),
		nextRequestBuffer_() {
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
	
	/** Receive headers from single request, the body may not completely received */
	seastar::future<> Http11ServerConnection::receiveSingleRequest() {
		if (state_ >= Http11ServerConnectionState::ReceiveRequestHeadersComplete) {
			// either headers completed or connection closing
			return seastar::make_ready_future<>();
		}
		// receive request headers
		seastar::future f = (nextRequestBuffer_.size() == 0 ?
			socket_.in().read() :
			seastar::make_ready_future<seastar::temporary_buffer<char>>(std::move(nextRequestBuffer_)));
		return std::move(f).then([this] (seastar::temporary_buffer<char> tempBuffer) {
			// store the last buffer received
			auto& lastBuffer = temporaryData_.lastBuffer;
			lastBuffer = std::move(tempBuffer);
			// check whether connection is closed from remote
			if (lastBuffer.size() == 0) {
				state_ = Http11ServerConnectionState::Closing;
				return seastar::make_ready_future<>();
			}
			// check bytes limitation of initial request data
			// no overflow check of receivedBytes because the buffer size should be small (up to 8192)
			// if receivedBytes + buffer size cause overflow that mean the limitation is too large
			temporaryData_.receivedBytes += lastBuffer.size();
			if (CPV_UNLIKELY(temporaryData_.receivedBytes >
				sharedData_->configuration.getMaxInitialRequestBytes())) {
				return replyStaticResponse(socket_, ReachedBytesLimitationOfInitialRequestData).then([] {
					return seastar::make_exception_future<>(LengthException(
						CPV_CODEINFO, "http request length error:",
						"reached bytes limitation of initial request data"));
				});
			}
			// check limitation of received packets, to avoid small packet attack
			temporaryData_.receivedPackets += 1;
			if (CPV_UNLIKELY(temporaryData_.receivedPackets >
				sharedData_->configuration.getMaxInitialRequestPackets())) {
				return replyStaticResponse(socket_, ReachedPacketsLimitationOfInitialRequestData).then([] {
					return seastar::make_exception_future<>(LengthException(
						CPV_CODEINFO, "http request length error:",
						"reached packets limitation of initial request data"));
				});
			}
			// execute http parser
			std::size_t parsedSize = ::http_parser_execute(
				&parser_,
				&parserSettings_,
				lastBuffer.get(),
				lastBuffer.size());
			if (parsedSize != lastBuffer.size()) {
				auto err = static_cast<enum ::http_errno>(parser_.http_errno);
				if (err == ::http_errno::HPE_CB_message_begin &&
					temporaryData_.messageCompleted) {
					// received next request from pipeline
					nextRequestBuffer_ = lastBuffer.share();
					nextRequestBuffer_.trim_front(parsedSize);
				} else {
					// parse error
					return replyErrorResponseForInvalidFormat();
				}
			}
			// hold underlying buffer in request
			request_.addUnderlyingBuffer(std::move(lastBuffer));
			// continue receiving
			return receiveSingleRequest();
		});
	}
	
	/** Reply single response and ensure the request body is completely received */
	seastar::future<> Http11ServerConnection::replySingleResponse() {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete ||
			state_ == Http11ServerConnectionState::ReceiveRequestBody ||
			state_ == Http11ServerConnectionState::ReceiveRequestMessageComplete) {
			// atleast all headers received, start replying response
			state_ = Http11ServerConnectionState::ReplyResponse;
			// call the first handler
			return sharedData_->handlers.front()->handle(
				request_, response_, sharedData_->handlers.begin() + 1).then([this] {
				// flush response headers
				// for response contains body, headers will flush from response stream
				// TODO
				// detect version and header to decide whether should keepalive or not
				// TODO
				// discard remain body from request
				// TODO
			});
		} else if (state_ == Http11ServerConnectionState::Closing) {
			// connection closing
			return seastar::make_ready_future<>();
		} else {
			// state error
			return replyStaticResponse(socket_, InvalidStateAfterReceivedSingleRequest).then([this] {
				return seastar::make_exception_future<>(LogicException(
					CPV_CODEINFO, "invalid state after received single request:", state_));
			});
		}
	}
	
	/** Send response headers if it's not sent previously */
	seastar::future<> Http11ServerConnection::flushResponseHeaders() {
		// caller should check responseHeadersFlushed before, so it's unlikely
		if (CPV_UNLIKELY(temporaryData_.responseHeadersFlushed)) {
			return seastar::make_ready_future<>();
		}
		// determine response http protocol version
		std::string_view version = response_.getVersion();
		if (CPV_LIKELY(version.empty())) {
			// copy version from request
			version = request_.getVersion();
			if (CPV_UNLIKELY(version.empty())) {
				// request version is unsupported
				version = constants::Http10;
			}
			// store version for determine keepalive later
			response_.setVersion(version);
		}
		// determine value of date header
		std::string_view date;
		auto& headers = response_.getHeaders();
		auto dateIt = headers.find(constants::Date);
		if (CPV_LIKELY(dateIt == headers.end())) {
			date = formatNowForHttpHeader();
		} else {
			date = dateIt->second;
			headers.erase(dateIt);
		}
		// determine value of server header
		std::string_view server;
		auto serverIt = headers.find(constants::Server);
		if (CPV_LIKELY(serverIt == headers.end())) {
			// no version number for security
			server = constants::CPVFramework;
		} else {
			server = serverIt->second;
			headers.erase(serverIt);
		}
		// calculate fragments count
		// +6: version, space, status code, space, status message, crlf
		// +4: date header, colon + space, header value, crlf
		// +4: server header, colon + space, header value, crlf
		// + headers count * 4
		// +1: crlf
		std::size_t fragmentsCount = 15 + headers.size() * 4;
		// build zero copy packet
		// TODO: test packet contains empty segment (empty header value)
		seastar::net::packet packet(fragmentsCount);
		packet << version << constants::Space <<
			response_.getStatusCode() << constants::Space <<
			response_.getStatusMessage() << constants::CRLF;
		packet << constants::Date << constants::ColonSpace <<
			date << constants::CRLF;
		packet << constants::Server << constants::ColonSpace <<
			server << constants::CRLF;
		for (auto& pair : response_.getHeaders()) {
			packet << pair.first << constants::ColonSpace <<
				pair.second << constants::CRLF;
		}
		packet << constants::CRLF;
		// send packet
		temporaryData_.responseHeadersFlushed = true;
		return socket_.out().write(std::move(packet));
	}
	
	/** Reply error response for invalid http request format, then return exception future */
	seastar::future<> Http11ServerConnection::replyErrorResponseForInvalidFormat() {
		return replyStaticResponse(socket_, InvalidHttpRequestFormat).then([this] {
			auto err = static_cast<enum ::http_errno>(parser_.http_errno);
			return seastar::make_exception_future<>(FormatException(
				CPV_CODEINFO, "http request format error:",
				::http_errno_name(err), ::http_errno_description(err),
				", state:", state_));
		});
	}
	
	int Http11ServerConnection::onMessageBegin(::http_parser* parser) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestInitial) {
			// normal begin
			self->state_ = Http11ServerConnectionState::ReceiveRequestMessageBegin;
		} else {
			// state error, maybe the next request from pipeline,
			// the caller should check parser_.http_errno and remember rest of the buffer
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::onUrl(::http_parser* parser, const char* data, std::size_t size) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestMessageBegin) {
			// the first time received the url
			self->state_ = Http11ServerConnectionState::ReceiveRequestUrl;
			self->temporaryData_.urlView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// url splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				self->temporaryData_.urlMerged,
				self->temporaryData_.urlView,
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
			flushMergedTemporaryBuffer(self->request_, self->temporaryData_.headerFieldMerged);
			flushMergedTemporaryBuffer(self->request_, self->temporaryData_.headerValueMerged);
			self->request_.setHeader(
				self->temporaryData_.headerFieldView,
				self->temporaryData_.headerValueView);
			self->temporaryData_.headerFieldView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// the first time received the first header field
			self->state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
			self->temporaryData_.headerFieldView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
			// header field splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				self->temporaryData_.headerFieldMerged,
				self->temporaryData_.headerFieldView,
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
			self->temporaryData_.headerValueView = std::string_view(data, size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// header value splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				self->temporaryData_.headerValueMerged,
				self->temporaryData_.headerValueView,
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
			flushMergedTemporaryBuffer(self->request_, self->temporaryData_.headerFieldMerged);
			flushMergedTemporaryBuffer(self->request_, self->temporaryData_.headerValueMerged);
			self->request_.setHeader(
				self->temporaryData_.headerFieldView,
				self->temporaryData_.headerValueView);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// no headers but url
			self->state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
		} else {
			// state error
			return -1;
		}
		// flush method, url and version
		flushMergedTemporaryBuffer(self->request_, self->temporaryData_.urlMerged);
		self->request_.setMethod(::http_method_str(
			static_cast<enum ::http_method>(self->parser_.method)));
		self->request_.setUrl(self->temporaryData_.urlView);
		self->request_.setVersion(getHttpVersionString(self->parser_));
		return 0;
	}
	
	int Http11ServerConnection::onBody(::http_parser* parser, const char* data, std::size_t size) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete) {
			// received initial body, share lastBuffer to bodyBuffer
			auto& lastBuffer = self->temporaryData_.lastBuffer;
			self->state_ = Http11ServerConnectionState::ReceiveRequestBody;
			self->temporaryData_.bodyBuffer = lastBuffer.share(data - lastBuffer.get(), size);
		} else if (self->state_ == Http11ServerConnectionState::ReceiveRequestBody) {
			// received initial chunked body, share lastBuffer to moreBodyBuffers
			auto& lastBuffer = self->temporaryData_.lastBuffer;
			self->temporaryData_.moreBodyBuffers.emplace_back(
				lastBuffer.share(data - lastBuffer.get(), size));
		} else if (self->state_ == Http11ServerConnectionState::ReplyResponse) {
			// receive body when replying response (called from request stream)
			auto& bodyBuffer = self->temporaryData_.bodyBuffer;
			if (bodyBuffer.size() == 0) {
				// move lastBuffer to bodyBuffer
				bodyBuffer = std::move(self->temporaryData_.lastBuffer);
			} else {
				// share bodyBuffer to moreBodyBuffers
				self->temporaryData_.moreBodyBuffers.emplace_back(
					bodyBuffer.share(data - bodyBuffer.get(), size));
			}
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::onMessageComplete(::http_parser* parser) {
		auto* self = reinterpret_cast<Http11ServerConnection*>(parser->data);
		if (self->state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete ||
			self->state_ == Http11ServerConnectionState::ReceiveRequestBody) {
			// no body or all body is received before reply
			self->state_ = Http11ServerConnectionState::ReceiveRequestMessageComplete;
			self->temporaryData_.messageCompleted = true;
		} else if (self->state_ == Http11ServerConnectionState::ReplyResponse) {
			// all body received when replying response (called from request stream)
			self->temporaryData_.messageCompleted = true;
		} else {
			// state error
			return -1;
		}
		return 0;
	}
}

