#include <seastar/core/sleep.hh>
#include <seastar/net/packet.hh>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Utility/PacketUtils.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include "./Http11ServerConnection.hpp"
#include "./Http11ServerConnectionRequestStream.hpp"
#include "./Http11ServerConnectionResponseStream.hpp"

/**
 * Implmentation Details:
 * 
 * For better pipeline support, this connection class will spawn two loop for
 * receive requests and reply responses, the receive loop will push requests to a queue once
 * headers competed (notice the body may not completed), and the reply loop will pop requests
 * from the queue and handle it.
 * 
 * Handle request body is a bit complicate, the receive loop will push body chunks to a queue,
 * this queue shared between all requests, request body stream have to pop body chunk from
 * the queue and make sure the body chunk belongs to the request.
 *
 * For less bugs, here are the rules for implementation:
 * - receive loop will not send any data to client
 * - receive loop only push request and body chunk to queue
 * - reply loop will not receive any data from client
 * - reply loop only pop request and body chunk from queue
 * - reply loop can send error to client after loop is end
 */

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
		}
		
		/**
		 * Override global setting of max header size in http_parser.
		 * The header size is checked in this framework, the parser library doesn't have to check it.
		 */
		static const bool HttpMaxHeaderSizeIsOverridden = ([] {
			#if HTTP_PARSER_VERSION_MAJOR > 2 || \
				(HTTP_PARSER_VERSION_MAJOR == 2 && HTTP_PARSER_VERSION_MINOR >= 9)
				internal::http_parser::http_parser_set_max_header_size(
					std::numeric_limits<std::uint32_t>::max());
				return true;
			#else
				return false;
			#endif
		})();
		
		/** Move temporary buffer to request if buffer not empty */
		static inline void addUnderlyingBufferIfNotEmpty(
			HttpRequest& request,
			seastar::temporary_buffer<char>& buffer) {
			if (CPV_UNLIKELY(buffer.size() > 0)) {
				request.addUnderlyingBuffer(std::move(buffer));
			}
		}
		
		/** Get http version string from parser, return empty for version not supported */
		static inline std::string_view getHttpVersionString(
			const internal::http_parser::http_parser& parser) {
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
			{ Http11ServerConnectionState::ReceiveRequestMessageBegin, "ReceiveRequestMessageBegin" },
			{ Http11ServerConnectionState::ReceiveRequestUrl, "ReceiveRequestUrl" },
			{ Http11ServerConnectionState::ReceiveRequestHeaderField, "ReceiveRequestHeaderField" },
			{ Http11ServerConnectionState::ReceiveRequestHeaderValue, "ReceiveRequestHeaderValue" },
			{ Http11ServerConnectionState::ReceiveRequestHeadersComplete, "ReceiveRequestHeadersComplete" },
			{ Http11ServerConnectionState::ReceiveRequestBody, "ReceiveRequestBody" },
			{ Http11ServerConnectionState::ReceiveRequestMessageComplete, "ReceiveRequestMessageComplete" },
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
		// spawn receive and reply loop
		// `this` will keep alive until `then` finished, so the loop can sure `this` is valid
		// receive loop and reply loop will handle exception on their own
		auto self = shared_from_this();
		seastar::when_all(startReceiveRequestLoop(), startReplyResponseLoop()).then(
			[self] (std::tuple<seastar::future<>, seastar::future<>>) {
			// cancel timer anyway in case of connection error
			self->shutdownInputTimer_.cancel();
			// remove self from connections collection (notice it's weak_ptr)
			auto* connectionsPtr = self->sharedData_->connectionsWrapper.get();
			std::size_t connectionsCount = 0;
			if (connectionsPtr != nullptr) {
				connectionsPtr->value.erase(self);
				connectionsCount = connectionsPtr->value.size();
				self->sharedData_->metricData.current_connections = connectionsCount;
			}
			// log and update state to closed
			self->sharedData_->logger->log(LogLevel::Info,
				"closed http connection from:", self->clientAddress_,
				", reason:", self->shutdownReason_,
				", remain connections count:", connectionsCount);
			self->state_ = Http11ServerConnectionState::Closed;
		});
	}
	
	/** Stop the connection immediately */
	seastar::future<> Http11ServerConnection::stop() {
		// shutdown connection
		shutdown("stop function called");
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
		shutdownInputTimer_(),
		requestQueue_(sharedData_->configuration.getRequestQueueSize()),
		requestBodyQueue_(sharedData_->configuration.getRequestBodyQueueSize()),
		newRequest_(),
		nextRequestBuffer_(),
		processingRequest_(),
		processingResponse_(),
		lastErrorResponse_(),
		shutdownReason_("not set"),
		parser_(),
		receiveLoopData_(),
		replyLoopData_() {
		// initialize timer
		shutdownInputTimer_.set_callback([this] {
			sharedData_->metricData.request_timeout_errors += 1;
			shutdown("request timeout");
		});
		// initialize http parser (will initialize again for next request)
		internal::http_parser::http_parser_init(&parser_, internal::http_parser::HTTP_REQUEST);
	}
	
	/** Shutdown connection, break receive and reply loop */
	void Http11ServerConnection::shutdown(const char* reason) {
		if (state_ == Http11ServerConnectionState::Closing) {
			return;
		}
		if (socket_.isConnected()) {
			// break receiving loop, keep output for error response
			socket_.socket().shutdown_input();
		}
		// break reply loop
		static thread_local std::exception_ptr ex(
			std::make_exception_ptr("abort queue for http connection shutdown"));
		requestQueue_.abort(ex);
		requestBodyQueue_.abort(ex);
		// update shutdown reason and state
		shutdownReason_ = reason;
		state_ = Http11ServerConnectionState::Closing;
	}
	
	/** start receiveRequestLoop and catch exceptions */
	seastar::future<> Http11ServerConnection::startReceiveRequestLoop() {
		return receiveRequestLoop().handle_exception([this] (std::exception_ptr ex) {
			if (state_ == Http11ServerConnectionState::Closing) {
				return;
			}
			sharedData_->metricData.request_errors += 1;
			sharedData_->logger->log(LogLevel::Info,
				"exception occurs when receive http request from", clientAddress_, ":", ex);
			shutdown("exception occurs when receive request");
		});
	}
	
	/** Keep receiving requests until state become closing, will push requests to a queue */
	seastar::future<> Http11ServerConnection::receiveRequestLoop() {
		// exit loop when closing connection
		if (state_ == Http11ServerConnectionState::Closing) {
			return seastar::make_ready_future<>();
		}
		// reset request data if previous request completed
		if (state_ == Http11ServerConnectionState::ReceiveRequestMessageComplete) {
			auto nextId = receiveLoopData_.requestId + 1;
			newRequest_ = {};
			receiveLoopData_ = {};
			receiveLoopData_.requestId = nextId;
			state_ = Http11ServerConnectionState::Started;
			internal::http_parser::http_parser_init(&parser_, internal::http_parser::HTTP_REQUEST);
		}
		// receive request headers or body
		// use custom timer instead of seastar::with_timeout for following reasons:
		// - with_timeout will allocate a new timer and the callback function each time
		// - delete connected_socket before read operation is finished will cause use-after-delete error
		shutdownInputTimer_.arm(seastar::timer<>::clock::now() +
			sharedData_->configuration.getRequestTimeout());
		seastar::future f = (nextRequestBuffer_.size() == 0 ?
			socket_.in().read() :
			seastar::make_ready_future<seastar::temporary_buffer<char>>(std::move(nextRequestBuffer_)));
		return f.then([this] (seastar::temporary_buffer<char> tempBuffer) {
			// cancel timer
			shutdownInputTimer_.cancel();
			// store the last buffer received
			auto& lastBuffer = receiveLoopData_.lastBuffer;
			lastBuffer = std::move(tempBuffer);
			// check whether connection is closed from remote
			if (lastBuffer.size() == 0) {
				shutdown("closed from remote");
				return seastar::make_ready_future<>();
			}
			if (state_ != Http11ServerConnectionState::ReceiveRequestBody) {
				// check bytes limitation of initial request data
				// no overflow check of receivedBytes because the buffer size should be small (up to 8192)
				// if receivedBytes + buffer size cause overflow that mean the limitation is too large
				receiveLoopData_.receivedBytes += lastBuffer.size();
				if (CPV_UNLIKELY(receiveLoopData_.receivedBytes >
					sharedData_->configuration.getMaxInitialRequestBytes())) {
					sharedData_->metricData.request_initial_size_errors += 1;
					lastErrorResponse_ = ReachedBytesLimitationOfInitialRequestData;
					shutdown("reached bytes limitation of initial request data");
					return seastar::make_ready_future<>();
				}
				// check limitation of received packets, to avoid small packet attack
				receiveLoopData_.receivedPackets += 1;
				if (CPV_UNLIKELY(receiveLoopData_.receivedPackets >
					sharedData_->configuration.getMaxInitialRequestPackets())) {
					sharedData_->metricData.request_initial_size_errors += 1;
					lastErrorResponse_ = ReachedPacketsLimitationOfInitialRequestData;
					shutdown("reached packets limitation of initial request data");
					return seastar::make_ready_future<>();
				}
			}
			// execute http parser
			std::size_t parsedSize = internal::http_parser::http_parser_execute(
				&parser_,
				this,
				lastBuffer.get(),
				lastBuffer.size());
			// check parse result
			if (parsedSize != lastBuffer.size()) {
				auto err = static_cast<enum internal::http_parser::http_errno>(parser_.http_errno);
				if (CPV_LIKELY(err == internal::http_parser::http_errno::HPE_CB_message_begin &&
					parsedSize > 1 &&
					state_ == Http11ServerConnectionState::ReceiveRequestMessageComplete)) {
					// received next request from pipeline
					nextRequestBuffer_ = lastBuffer.share();
					nextRequestBuffer_.trim_front(parsedSize - 1);
				} else {
					// parse error, log level is info because it may not cause by server
					sharedData_->metricData.request_invalid_format_errors += 1;
					lastErrorResponse_ = InvalidHttpRequestFormat;
					sharedData_->logger->log(LogLevel::Info,
						"http request format error from client", clientAddress_,
						", error:", internal::http_parser::http_errno_name(err),
						", description:", internal::http_parser::http_errno_description(err),
						", state:", state_);
					shutdown("invalid request format");
					return seastar::make_ready_future<>();
				}
			}
			// add underlying buffer to request
			// if request is enqueued, then lastBuffer is shared to bodyBuffers
			if (CPV_LIKELY(!receiveLoopData_.requestEnqueued)) {
				newRequest_.addUnderlyingBuffer(std::move(lastBuffer));
			}
			// enqueue body buffers => enqueue request when headers completed => continue receiving
			if (receiveLoopData_.bodyBuffers.empty()) {
				return enqueueRequestAndContinueReceiving(false);
			} else {
				return enqueueBodyBuffers().then([this] {
					return enqueueRequestAndContinueReceiving(true);
				});
			}
		});
	}
	
	/** (for receive loop) Enqueue body buffers to queue */
	seastar::future<> Http11ServerConnection::enqueueBodyBuffers() {
		if (receiveLoopData_.bodyBufferEnqueueIndex >= receiveLoopData_.bodyBuffers.size()) {
			receiveLoopData_.bodyBufferEnqueueIndex = 0;
			receiveLoopData_.bodyBuffers.clear();
			return seastar::make_ready_future<>();
		}
		std::size_t index = receiveLoopData_.bodyBufferEnqueueIndex++;
		BodyEntry entry({
			std::move(receiveLoopData_.bodyBuffers[index]),
			receiveLoopData_.requestId,
			(index + 1 == receiveLoopData_.bodyBuffers.size() &&
				state_ == Http11ServerConnectionState::ReceiveRequestMessageComplete)
		});
		return requestBodyQueue_.push_eventually(std::move(entry)).then([this] {
			return enqueueBodyBuffers();
		});
	}
	
	/** (for receive loop) Enqueue request to queue when headers completed, and continue receiving */
	seastar::future<> Http11ServerConnection::enqueueRequestAndContinueReceiving(bool hasBody) {
		if (!receiveLoopData_.requestEnqueued &&
			(state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete ||
			state_ == Http11ServerConnectionState::ReceiveRequestBody ||
			state_ == Http11ServerConnectionState::ReceiveRequestMessageComplete)) {
			// headers completed, enqueue request to queue
			sharedData_->metricData.request_received += 1;
			receiveLoopData_.requestEnqueued = true;
			RequestEntry entry({
				std::move(newRequest_),
				receiveLoopData_.requestId,
				hasBody || state_ != Http11ServerConnectionState::ReceiveRequestMessageComplete
			});
			auto f = requestQueue_.push_eventually(std::move(entry));
			if (CPV_UNLIKELY(!f.available())) {
				return f.then([this] {
					return receiveRequestLoop();
				});
			}
		}
		return receiveRequestLoop();
	}
	
	/** start replyResponseLoop and catch exceptions */
	seastar::future<> Http11ServerConnection::startReplyResponseLoop() {
		return replyResponseLoop().handle_exception([this] (std::exception_ptr ex) {
			if (state_ == Http11ServerConnectionState::Closing) {
				return replyResponseLoop(); // send error response
			}
			sharedData_->metricData.request_errors += 1;
			sharedData_->logger->log(LogLevel::Info,
				"exception occurs when reply http response to", clientAddress_, ":", ex);
			shutdown("exception occurs when reply response");
			return replyResponseLoop(); // send error response
		});
	}
	
	/** Keep reply responses until state become closing, will pop requests from the queue */
	seastar::future<> Http11ServerConnection::replyResponseLoop() {
		// exit loop when closing connection
		if (state_ == Http11ServerConnectionState::Closing) {
			// send error response to client if there any
			if (CPV_LIKELY(lastErrorResponse_.empty() || !socket_.isConnected())) {
				return seastar::make_ready_future<>();
			}
			return socket_.out()
				.put(seastar::net::packet::from_static_data(
					lastErrorResponse_.data(), lastErrorResponse_.size()))
				.then([this] { socket_.out().flush(); })
				.handle_exception([] (std::exception_ptr) { });
		}
		// pop request from queue
		return requestQueue_.pop_eventually().then([this] (RequestEntry entry) {
			// setup request and response
			processingRequest_ = std::move(entry.request);
			processingResponse_ = {};
			processingRequest_.setBodyStream(
				makeObject<Http11ServerConnectionRequestStream>(this).cast<InputStreamBase>());
			processingResponse_.setBodyStream(
				makeObject<Http11ServerConnectionResponseStream>(this).cast<OutputStreamBase>());
			replyLoopData_ = {};
			replyLoopData_.requestId = entry.id;
			replyLoopData_.requestBodyConsumed = !entry.hasBody;
			// invoke the first handler
			return sharedData_->handlers.front()->handle(
				processingRequest_,
				processingResponse_,
				sharedData_->handlers.begin() + 1).then([this] {
				// send response headers if not sent before, and flush response
				seastar::future<> result = seastar::make_ready_future<>();
				if (replyLoopData_.responseHeadersAppended) {
					result = socket_.out().flush();
				} else {
					seastar::net::packet data(getResponseHeadersFragmentsCount());
					appendResponseHeaders(data);
					result = socket_.out().put(std::move(data)).then([this] {
						return socket_.out().flush();
					});
				}
				// check keepalive again
				if (CPV_LIKELY(replyLoopData_.keepConnection)) {
					replyLoopData_.keepConnection = checkKeepaliveByContentLength();
				}
				// handle next request if keepalive enabled
				if (CPV_LIKELY(replyLoopData_.keepConnection)) {
					if (CPV_LIKELY(result.available())) {
						return replyResponseLoop(); // hot path
					} else {
						return result.then([this] {
							return replyResponseLoop();
						});
					}
				} else {
					return result.then([this] {
						shutdown("keepalive not enabled");
						return replyResponseLoop();
					});
				}
			});
		});
	}
	
	/** (for reply loop) Get maximum fragments count for response headers */
	std::size_t Http11ServerConnection::getResponseHeadersFragmentsCount() const {
		// calculate fragments count
		// +6: version, space, status code, space, status message, crlf
		// +4: date header, colon + space, header value, crlf
		// +4: server header, colon + space, header value, crlf
		// +4: connection header, colon + space, header value, crlf
		// + headers count * 4
		// +1: crlf
		return 19 + processingResponse_.getHeaders().maxSize() * 4;
	}
	
	/** (for reply loop) Append response headers to packet, please check responseHeadersAppended first */
	void Http11ServerConnection::appendResponseHeaders(seastar::net::packet& packet) {
		// check flag
		if (CPV_UNLIKELY(replyLoopData_.responseHeadersAppended)) {
			return;
		}
		replyLoopData_.responseHeadersAppended = true;
		// set protocol version
		std::string_view version = processingResponse_.getVersion();
		if (CPV_LIKELY(version.empty())) {
			// copy version from request
			version = processingRequest_.getVersion();
			if (CPV_UNLIKELY(version.empty())) {
				// request version is unsupported
				version = constants::Http10;
			}
			// store version for keepalive determination later
			processingResponse_.setVersion(version);
		}
		// return a special status code for handler didn't set it
		if (CPV_UNLIKELY(
			processingResponse_.getStatusCode().empty() ||
			processingResponse_.getStatusMessage().empty())) {
			processingResponse_.setStatusCode("0");
			processingResponse_.setStatusMessage("Status code or status message not set");
		}
		// set date header
		auto& responseHeaders = processingResponse_.getHeaders();
		if (CPV_LIKELY(responseHeaders.getDate().empty())) {
			responseHeaders.setDate(formatNowForHttpHeader());
		}
		// set server header
		if (CPV_LIKELY(responseHeaders.getServer().empty())) {
			// no version number for security
			responseHeaders.setServer(constants::CPVFramework);
		}
		// set connection header
		replyLoopData_.keepConnection = checkKeepaliveByConnnectionHeader();
		auto connectionValue = responseHeaders.getConnection();
		if (CPV_LIKELY(connectionValue.empty())) {
			if (CPV_LIKELY(replyLoopData_.keepConnection)) {
				responseHeaders.setConnection(constants::Keepalive);
			} else {
				responseHeaders.setConnection(constants::Close);
			}
		} else {
			// custom connection header, close connection if it isn't keep-alive
			if (CPV_LIKELY(connectionValue != constants::Keepalive)) {
				replyLoopData_.keepConnection = false;
			}
		}
		// append response headers to packet
		packet << version << constants::Space <<
			processingResponse_.getStatusCode() << constants::Space <<
			processingResponse_.getStatusMessage() << constants::CRLF;
		responseHeaders.foreach([&packet] (const auto& key, const auto& value) {
			packet << key << constants::ColonSpace << value << constants::CRLF;
		});
		packet << constants::CRLF;
	}
	
	/** (for reply loop) Determine whether keep connection or not by checking connection header */
	bool Http11ServerConnection::checkKeepaliveByConnnectionHeader() const {
		auto& requestHeaders = processingRequest_.getHeaders();
		auto connectionValue = requestHeaders.getConnection();
		if (CPV_LIKELY(connectionValue == constants::Keepalive)) {
			// client wants to keepalive
			// most browser will send connection header even for http 1.1
			return true;
		} else if (!connectionValue.empty()) {
			// client doesn't want to keepalive
			// it may be "close" or other unsupported string literal
			return false;
		} else if (processingResponse_.getVersion() == constants::Http10) {
			// for http 1.0, keepalive is disabled by default
			return false;
		} else {
			// for http > 1.0, keepalive is enabled by default
			return true;
		}
	}
	
	/** (for reply loop) Determine whether keep connection or not by checking content length */
	bool Http11ServerConnection::checkKeepaliveByContentLength() const {
		// check whether content length is fixed or chunked
		auto& responseHeaders = processingResponse_.getHeaders();
		auto contentLengthValue = responseHeaders.getContentLength();
		if (CPV_UNLIKELY(contentLengthValue.empty())) {
			// content length did not set, check transfer encoding
			if (responseHeaders.getTransferEncoding() != constants::Chunked) {
				// close connection to indicate response is end
				return false;
			}
		} else {
			// check whether content length of response is matched to written size
			std::size_t contentLength = 0;
			if (CPV_UNLIKELY(!loadIntFromDec(
				contentLengthValue.data(), contentLengthValue.size(), contentLength))) {
				sharedData_->logger->log(LogLevel::Warning,
					"going to close inconsistent connection from", clientAddress_,
					"because content length of response isn't integer");
				return false;
			}
			if (CPV_UNLIKELY(contentLength != replyLoopData_.responseWrittenBytes)) {
				sharedData_->logger->log(LogLevel::Warning,
					"going to close inconsistent connection from", clientAddress_,
					"because content length of response doesn't matched to written size");
				return false;
			}
		}
		// check whether request content is completely consumed
		if (CPV_UNLIKELY(!replyLoopData_.requestBodyConsumed)) {
			// close connection to discard remain parts
			return false;
		}
		// request content is completely consumed,
		// and response content is written with fixed size or chunked encoding
		return true;
	}
	
	int Http11ServerConnection::on_message_begin() {
		if (state_ == Http11ServerConnectionState::Started) {
			// normal begin
			state_ = Http11ServerConnectionState::ReceiveRequestMessageBegin;
		} else {
			// state error, maybe the next request from pipeline,
			// the caller should check parser_.http_errno and remember rest of the buffer
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_url(const char* data, std::size_t size) {
		if (state_ == Http11ServerConnectionState::ReceiveRequestMessageBegin) {
			// the first time received the url
			state_ = Http11ServerConnectionState::ReceiveRequestUrl;
			receiveLoopData_.urlView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// url splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				receiveLoopData_.urlMerged,
				receiveLoopData_.urlView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_header_field(const char* data, std::size_t size) {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// the first time received a new header field, store last header field and value
			state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
			addUnderlyingBufferIfNotEmpty(newRequest_, receiveLoopData_.headerFieldMerged);
			addUnderlyingBufferIfNotEmpty(newRequest_, receiveLoopData_.headerValueMerged);
			newRequest_.setHeader(
				receiveLoopData_.headerFieldView,
				receiveLoopData_.headerValueView);
			receiveLoopData_.headerFieldView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// the first time received the first header field
			state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
			receiveLoopData_.headerFieldView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
			// header field splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				receiveLoopData_.headerFieldMerged,
				receiveLoopData_.headerFieldView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_header_value(const char* data, std::size_t size) {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
			// the first time received a header value after header field
			state_ = Http11ServerConnectionState::ReceiveRequestHeaderValue;
			receiveLoopData_.headerValueView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// header value splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				receiveLoopData_.headerValueMerged,
				receiveLoopData_.headerValueView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_headers_complete() {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// all headers received, store last header field and value
			state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
			addUnderlyingBufferIfNotEmpty(newRequest_, receiveLoopData_.headerFieldMerged);
			addUnderlyingBufferIfNotEmpty(newRequest_, receiveLoopData_.headerValueMerged);
			newRequest_.setHeader(
				receiveLoopData_.headerFieldView,
				receiveLoopData_.headerValueView);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// no headers but url
			state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
		} else {
			// state error
			return -1;
		}
		// store method, url and version
		addUnderlyingBufferIfNotEmpty(newRequest_, receiveLoopData_.urlMerged);
		newRequest_.setMethod(internal::http_parser::http_method_str(
			static_cast<enum internal::http_parser::http_method>(parser_.method)));
		newRequest_.setUrl(receiveLoopData_.urlView);
		newRequest_.setVersion(getHttpVersionString(parser_));
		return 0;
	}
	
	int Http11ServerConnection::on_body(const char* data, std::size_t size) {
		// warning:
		// newRequest_ maybe empty (moved to queue but not all completed), don't touch it
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete) {
			// received initial body, share lastBuffer to bodyBuffers
			state_ = Http11ServerConnectionState::ReceiveRequestBody;
			auto& lastBuffer = receiveLoopData_.lastBuffer;
			receiveLoopData_.bodyBuffers.emplace_back(
				lastBuffer.share(data - lastBuffer.get(), size));
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestBody) {
			// received following body, share lastBuffer to bodyBuffers
			auto& lastBuffer = receiveLoopData_.lastBuffer;
			receiveLoopData_.bodyBuffers.emplace_back(
				lastBuffer.share(data - lastBuffer.get(), size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_message_complete() {
		// warning:
		// newRequest_ maybe empty (moved to queue but not all completed), don't touch it
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete ||
			state_ == Http11ServerConnectionState::ReceiveRequestBody) {
			// no body or all body received
			if (receiveLoopData_.requestEnqueued && receiveLoopData_.bodyBuffers.empty()) {
				// request enqueued before completed, but no body received
				// add empty tail body to make sure stream can finish
				receiveLoopData_.bodyBuffers.emplace_back();
			}
			state_ = Http11ServerConnectionState::ReceiveRequestMessageComplete;
		} else {
			// state error
			return -1;
		}
		return 0;
	}
}

