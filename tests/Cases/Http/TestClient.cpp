#include <core/semaphore.hh>
#include <core/gate.hh>
#include <CPVFramework/Http/Client.hpp>
#include <CPVFramework/Http/Server.hpp>
#include <CPVFramework/Http/Route.hpp>
#include <CPVFramework/Http/Handler.hpp>
#include <TestUtility/GTestUtils.hpp>
#include <Http/Client/HttpClientRequestImpl.hpp>
#include <Http/Client/HttpClientResponseImpl.hpp>

TEST(TestHttpClient, buildRequest) {
	{
		auto request = cpv::makeObject<cpv::HttpClientRequestImpl>(
			"GET", "/index.html", "localhost:8000");
		request->addHeader("User-Agent", "someClient");
		request->setBody("text/html", "<html></html>");
		ASSERT_EQ(request->str(),
			"GET /index.html HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"User-Agent: someClient\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"<html></html>");
	}
	{
		auto request = cpv::makeObject<cpv::HttpClientRequestImpl>(
			"GET", "/index.html", "localhost:8000");
		request->setBody("plain/text", "abc");
		ASSERT_EQ(request->str(),
			"GET /index.html HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"User-Agent: cpv-http-client " CPV_FRAMEWORK_VERSION_NUMBER "\r\n"
			"Content-Type: plain/text\r\n"
			"Content-Length: 3\r\n"
			"\r\n"
			"abc");
	}
}

TEST(TestHttpClient, parseResponse) {
	for (std::size_t i = 0; i < 3; ++i) {
		auto response = cpv::makeObject<cpv::HttpClientResponseImpl>();
		ASSERT_FALSE(response->appendReceived("HTTP/1.1 200 OK\r\nServer: "));
		ASSERT_FALSE(response->appendReceived("Apache/2.4\r\nContent-Length: 13\r\nContent-"));
		ASSERT_FALSE(response->appendReceived("Type: text/html\r\n\r\n<html>"));
		ASSERT_TRUE(response->appendReceived("</html>"));
		ASSERT_EQ(response->getStatus(), 200);
		ASSERT_EQ(response->getHeader("Server"), "Apache/2.4");
		ASSERT_EQ(response->getHeader("Content-Length"), "13");
		ASSERT_EQ(response->getHeader("Content-Type"), "text/html");
		ASSERT_TRUE(response->getHeader("NotExist").empty());
		ASSERT_EQ(response->getBody(), "<html></html>");
	}
}

TEST(TestHttpClient, factory) {
	cpv::HttpClientFactoryImpl clientFactory;
	auto a = clientFactory.create("localhost", 8000);
	auto b = clientFactory.create("localhost", 8000);
	auto c = clientFactory.create("localhost_", 8000);
	auto d = clientFactory.create("localhost", 8001);
	ASSERT_EQ(a.get(), b.get());
	ASSERT_NE(a.get(), c.get());
	ASSERT_NE(a.get(), d.get());
	ASSERT_NE(c.get(), d.get());
}

TEST(TestHttpClient, factoryError) {
	{
		cpv::HttpClientFactoryImpl clientFactory;
		clientFactory.create("localhost", 8000);
		ASSERT_THROWS_CONTAINS(
			cpv::LogicException,
			clientFactory.createSSL("localhost", 8000, "", ""),
			"already created and use plain http");
	}
	{
		cpv::HttpClientFactoryImpl clientFactory;
		clientFactory.createSSL("localhost", 8000, "", "");
		ASSERT_THROWS_CONTAINS(
			cpv::LogicException,
			clientFactory.create("localhost", 8000),
			"already created and use https");
	}
}

TEST_FUTURE(TestHttpClient, httpClient) {
	auto server = seastar::make_shared<cpv::httpd::http_server_control>();
	return server->start().then([server] {
		return server->set_routes([] (cpv::httpd::routes& r) {
			r.put(
				cpv::httpd::operation_type::GET,
				"/hello",
				new cpv::httpd::function_handler([] (cpv::httpd::const_req req) {
					return "hello world";
				}, "txt"));
		});
	}).then([server] {
		return server->listen({ HTTPD_LISTEN_IP, HTTPD_LISTEN_PORT });
	}).then([] {
		cpv::HttpClientFactoryImpl clientFactory;
		return seastar::do_with(
			seastar::semaphore(20),
			seastar::gate(),
			100UL,
			clientFactory.create("127.0.0.1", HTTPD_LISTEN_PORT),
			[] (auto& limit, auto& gate, auto& count, auto& client) {
			return seastar::repeat([&limit, &gate, &count, &client] {
				return seastar::get_units(limit, 1).then([&gate, &count, &client] (auto units) {
					gate.enter();
					auto request = client->makeRequest("GET", "/hello");
					request->setBody("", "");
					client->send(std::move(request)).then([] (auto response) {
						ASSERT_EQ(response->getStatus(), 200);
						ASSERT_EQ(response->getHeader("Server"), "Seastar httpd");
						ASSERT_EQ(response->getBody(), "hello world");
					}).finally([&gate, units = std::move(units)] {
						gate.leave();
					});
				}).then([&count] {
					return --count == 0 ?
						seastar::stop_iteration::yes :
						seastar::stop_iteration::no;
				});
			}).finally([&gate] {
				return gate.close();
			});;
		});
	}).finally([server] {
		return server->stop();
	}).finally([server] { });
}

