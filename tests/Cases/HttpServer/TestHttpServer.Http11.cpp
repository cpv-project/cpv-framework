#include <algorithm>
#include <atomic>
#include <seastar/core/future-util.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/Utility/PacketUtils.hpp>
#include <TestUtility/GTestUtils.hpp>
#include "./TestHttpServer.Base.hpp"

TEST_FUTURE(HttpServer_Http11, simple) {
	cpv::gtest::HttpServerTestFunctions testFunctions;
	testFunctions.makeHandlers = [] {
		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(std::make_unique<cpv::gtest::HttpCheckHeadersHandler>());
		return handlers;
	};
	testFunctions.execute = [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "GET /test_headers HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: close\r\n"
				"Content-Length: 160\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n\r\n"
				"request method: GET\r\n"
				"request url: /test_headers\r\n"
				"request version: HTTP/1.1\r\n"
				"request headers:\r\n"
				"  Connection: close\r\n"
				"  Host: localhost\r\n"
				"  User-Agent: TestClient\r\n");
		});
	};
	return cpv::gtest::runHttpServerTest(std::move(testFunctions));
}

TEST_FUTURE(HttpServer_Http11, simpleWithBody) {
	cpv::gtest::HttpServerTestFunctions testFunctions;
	testFunctions.makeHandlers = [] {
		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(std::make_unique<cpv::gtest::HttpCheckBodyHandler>());
		return handlers;
	};
	testFunctions.execute = [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "GET /test_body HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"Content-Length: 11\r\n"
			"Content-Type: application/octet-stream\r\n"
			"User-Agent: TestClient\r\n\r\n"
			"Hello World";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: close\r\n"
				"Content-Length: 11\r\n"
				"Content-Type: application/octet-stream\r\n\r\n"
				"Hello World");
		});
	};
	return cpv::gtest::runHttpServerTest(std::move(testFunctions));
}

TEST_FUTURE(HttpServer_Http11, pipeline) {
	cpv::gtest::HttpServerTestFunctions testFunctions;
	testFunctions.makeHandlers = [] {
		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(std::make_unique<cpv::gtest::HttpCheckHeadersHandler>());
		return handlers;
	};
	testFunctions.execute = [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "GET /test_first HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: keep-alive\r\n"
			"User-Agent: TestClient First\r\n\r\n"
			"GET /test_second HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient Second\r\n\r\n";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 169\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n\r\n"
				"request method: GET\r\n"
				"request url: /test_first\r\n"
				"request version: HTTP/1.1\r\n"
				"request headers:\r\n"
				"  Connection: keep-alive\r\n"
				"  Host: localhost\r\n"
				"  User-Agent: TestClient First\r\n"
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: close\r\n"
				"Content-Length: 166\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n\r\n"
				"request method: GET\r\n"
				"request url: /test_second\r\n"
				"request version: HTTP/1.1\r\n"
				"request headers:\r\n"
				"  Connection: close\r\n"
				"  Host: localhost\r\n"
				"  User-Agent: TestClient Second\r\n");
		});
	};
	return cpv::gtest::runHttpServerTest(std::move(testFunctions));
}

TEST_FUTURE(HttpServer_Http11, pipelineWithBody) {
	cpv::gtest::HttpServerTestFunctions testFunctions;
	testFunctions.makeHandlers = [] {
		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(std::make_unique<cpv::gtest::HttpCheckBodyHandler>());
		return handlers;
	};
	testFunctions.execute = [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "GET /test_first HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: 17\r\n"
			"Content-Type: application/octet-stream\r\n"
			"User-Agent: TestClient\r\n\r\n"
			"Hello World First"
			"GET /test_second HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"Content-Length: 18\r\n"
			"Content-Type: application/octet-stream\r\n"
			"User-Agent: TestClient\r\n\r\n"
			"Hello World Second";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 17\r\n"
				"Content-Type: application/octet-stream\r\n\r\n"
				"Hello World First"
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: close\r\n"
				"Content-Length: 18\r\n"
				"Content-Type: application/octet-stream\r\n\r\n"
				"Hello World Second");
		});
	};
	return cpv::gtest::runHttpServerTest(std::move(testFunctions));
}

TEST_FUTURE(HttpServer_Http11, chunkedBody) {
	cpv::gtest::HttpServerTestFunctions testFunctions;
	testFunctions.makeHandlers = [] {
		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(std::make_unique<cpv::gtest::HttpCheckBodyHandler>());
		return handlers;
	};
	testFunctions.execute = [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "GET /test_chunked_body HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"Content-Type: application/octet-stream\r\n"
			"User-Agent: TestClient\r\n"
			"Transfer-Encoding: chunked\r\n\r\n"
			"C\r\n"
			"Hello World \r\n"
			"7\r\n"
			"Chunked\r\n"
			"0\r\n"
			"\r\n";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: close\r\n"
				"Content-Length: 19\r\n"
				"Content-Type: application/octet-stream\r\n\r\n"
				"Hello World Chunked");
		});
	};
	return cpv::gtest::runHttpServerTest(std::move(testFunctions));
}

TEST_FUTURE(HttpServer_Http11, pipelineWithChunkedBody) {
	cpv::gtest::HttpServerTestFunctions testFunctions;
	testFunctions.makeHandlers = [] {
		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(std::make_unique<cpv::gtest::HttpCheckBodyHandler>());
		return handlers;
	};
	testFunctions.execute = [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "GET /test_first HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type: application/octet-stream\r\n"
			"User-Agent: TestClient\r\n"
			"Transfer-Encoding: chunked\r\n\r\n"
			"C\r\n"
			"Hello World \r\n"
			"8\r\n"
			"Chunked \r\n"
			"5\r\n"
			"First\r\n"
			"0\r\n"
			"\r\n"
			"GET /test_second HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"Content-Type: application/octet-stream\r\n"
			"User-Agent: TestClient\r\n"
			"Transfer-Encoding: chunked\r\n\r\n"
			"C\r\n"
			"Hello World \r\n"
			"8\r\n"
			"Chunked \r\n"
			"6\r\n"
			"Second\r\n"
			"0\r\n"
			"\r\n";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 25\r\n"
				"Content-Type: application/octet-stream\r\n\r\n"
				"Hello World Chunked First"
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: close\r\n"
				"Content-Length: 26\r\n"
				"Content-Type: application/octet-stream\r\n\r\n"
				"Hello World Chunked Second");
		});
	};
	return cpv::gtest::runHttpServerTest(std::move(testFunctions));
}

TEST_FUTURE(HttpServer_Http11, keepaliveTimeout) {
	cpv::gtest::HttpServerTestFunctions testFunctions;
	testFunctions.updateConfiguration = [] (cpv::HttpServerConfiguration& configuration) {
		configuration.setInitialRequestTimeout(std::chrono::milliseconds(100));
	};
	testFunctions.makeHandlers = [] {
		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(std::make_unique<cpv::gtest::HttpCheckBodyHandler>());
		return handlers;
	};
	testFunctions.execute = [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "POST /test_first HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: 17\r\n"
			"Content-Type: text/plain\r\n"
			"User-Agent: TestClient\r\n\r\n"
			"Hello World First"
			"POST /test_second HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: 18\r\n"
			"Content-Type: text/plain\r\n"
			"User-Agent: TestClient\r\n\r\n"
			"Hello World Second"
			"POST /test_third HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: 17\r\n"
			"Content-Type: text/plain\r\n"
			"User-Agent: TestClient\r\n\r\n"
			"Hello World Third";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 17\r\n"
				"Content-Type: text/plain\r\n\r\n"
				"Hello World First"
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 18\r\n"
				"Content-Type: text/plain\r\n\r\n"
				"Hello World Second"
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 17\r\n"
				"Content-Type: text/plain\r\n\r\n"
				"Hello World Third");
		});
	};
	return cpv::gtest::runHttpServerTest(std::move(testFunctions));
}

TEST_FUTURE(HttpServer_Http11, partial) {
	// TODO
	return seastar::make_ready_future<>();
}

TEST_FUTURE(HttpServer_Http11, maxInitialRequestBytes) {
	// TODO
	return seastar::make_ready_future<>();
}

TEST_FUTURE(HttpServer_Http11, maxInitialRequestPackets) {
	// TODO
	return seastar::make_ready_future<>();
}

TEST_FUTURE(HttpServer_Http11, invalidFormat) {
	// TODO
	return seastar::make_ready_future<>();
}

