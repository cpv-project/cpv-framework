#pragma once
#include <http_parser.h>
#include <seastar/core/timer.hh>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp>
#include "../HttpServerSharedData.hpp"
#include "./HttpServerConnectionBase.hpp"

namespace cpv {
	/** The state of a http 1.0/1.1 connection */
	enum class Http11ServerConnectionState {
		Initial,
		Started,
		ReceiveRequestInitial,
		ReceiveRequestMessageBegin,
		ReceiveRequestUrl,
		ReceiveRequestHeaderField,
		ReceiveRequestHeaderValue,
		ReceiveRequestHeadersComplete,
		ReceiveRequestBody,
		ReceiveRequestMessageComplete,
		ReplyResponse,
		Closing,
		Closed
	};
	
	/** Enum descriptions of Http11ServerConnectionState */
	template <>
	struct EnumDescriptions<Http11ServerConnectionState> {
		static const std::vector<std::pair<Http11ServerConnectionState, const char*>>& get();
	};
	
	/** Connection accepted from http client uses http 1.0/1.1 protocol */
	class Http11ServerConnection :
		public HttpServerConnectionBase,
		public seastar::enable_shared_from_this<Http11ServerConnection> {
	public:
		/** Start receive requests and send responses */
		void start();
		
		/** Stop the connection immediately */
		seastar::future<> stop() override;
		
		/** Constructor */
		Http11ServerConnection(
			const seastar::lw_shared_ptr<HttpServerSharedData>& sharedData,
			seastar::connected_socket&& fd,
			seastar::socket_address&& addr);
	
	private:
		/** Receive headers from single request, the body may not completely received */
		seastar::future<> receiveSingleRequest();
		
		/** Reply single response and ensure the request body is completely received */
		seastar::future<> replySingleResponse();
		
		/** Send response headers if it's not sent previously */
		seastar::future<> flushResponseHeaders();
		
		/** Reply error response for invalid http request format, then return exception future */
		seastar::future<> replyErrorResponseForInvalidFormat();
		
		/** Parser callbacks */
		static int onMessageBegin(::http_parser* parser);
		static int onUrl(::http_parser* parser, const char* data, std::size_t size);
		static int onHeaderField(::http_parser* parser, const char* data, std::size_t size);
		static int onHeaderValue(::http_parser* parser, const char* data, std::size_t size);
		static int onHeadersComplete(::http_parser* parser);
		static int onBody(::http_parser* parser, const char* data, std::size_t size);
		static int onMessageComplete(::http_parser* parser);
		static int onChunkHeader(::http_parser* parser);
		static int onChunkComplete(::http_parser* parser);
		
		/** Friends **/
		friend class Http11ServerConnectionRequestStream;
		friend class Http11ServerConnectionResponseStream;
	
	private:
		seastar::lw_shared_ptr<HttpServerSharedData> sharedData_;
		SocketHolder socket_;
		seastar::socket_address clientAddress_;
		Http11ServerConnectionState state_;
		HttpRequest request_;
		HttpResponse response_;
		::http_parser_settings parserSettings_;
		::http_parser parser_;
		// temporary data for single request, store in unnamed struct for easily reset
		struct {
			// the last buffer received, store from receiveSingleRequest or request stream
			seastar::temporary_buffer<char> lastBuffer;
			// for initial request size check
			std::size_t receivedBytes = 0;
			std::size_t receivedPackets = 0;
			// for url splited in multiple packets
			seastar::temporary_buffer<char> urlMerged;
			std::string_view urlView;
			// for header field splited in multiple packets
			seastar::temporary_buffer<char> headerFieldMerged;
			std::string_view headerFieldView;
			// for header value splited in multiple packets
			seastar::temporary_buffer<char> headerValueMerged;
			std::string_view headerValueView;
			// for body, may splited as multiple parts if encoding is chunked
			seastar::temporary_buffer<char> bodyBuffer;
			StackAllocatedVector<seastar::temporary_buffer<char>, 16> moreBodyBuffers;
			// is message completed (no body or all body received)
			bool messageCompleted = false;
			// is response headers sent previously
			bool responseHeadersFlushed = false;
			// bytes of response body written to client
			std::size_t responseBodyWrittenSize = 0;
			// is connection kept for next request
			bool keepConnection = false;
		} temporaryData_;
		// the rest of buffer for next request received from pipeline
		seastar::temporary_buffer<char> nextRequestBuffer_;
		// the timer use to implement initial request timeout
		seastar::timer<> shutdownInputTimer_;
	};
}

