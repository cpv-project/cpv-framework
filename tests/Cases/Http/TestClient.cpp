#include <core/semaphore.hh>
#include <core/gate.hh>
#include <CPVFramework/Http/Client.hpp>
#include <CPVFramework/Http/Server.hpp>
#include <CPVFramework/Http/Route.hpp>
#include <CPVFramework/Http/Handler.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestHttpClient, buildRequest) {
	{
		auto request = cpv::HttpClientRequest("GET", "/index.html", "localhost:8000")
			.addHeader("User-Agent", "someClient")
			.setBody("text/html", "<html></html>");
		ASSERT_EQ(request.str(),
			"GET /index.html HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"User-Agent: someClient\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"<html></html>");
	}
	{
		auto request = cpv::HttpClientRequest("GET", "/index.html", "localhost:8000")
			.setBody("plain/text", "abc");
		ASSERT_EQ(request.str(),
			"GET /index.html HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"User-Agent: cpv-http-client\r\n"
			"Content-Type: plain/text\r\n"
			"Content-Length: 3\r\n"
			"\r\n"
			"abc");
	}
}

TEST(TestHttpClient, parseResponse) {
	cpv::HttpClientResponse response;
	ASSERT_FALSE(response.appendReceived("HTTP/1.1 200 OK\r\nServer: "));
	ASSERT_FALSE(response.appendReceived("Apache/2.4\r\nContent-Length: 13\r\nContent-"));
	ASSERT_FALSE(response.appendReceived("Type: text/html\r\n\r\n<html>"));
	ASSERT_TRUE(response.appendReceived("</html>"));
	ASSERT_EQ(response.getStatus(), 200);
	ASSERT_EQ(response.getHeader("Server"), "Apache/2.4");
	ASSERT_EQ(response.getHeader("Content-Length"), "13");
	ASSERT_EQ(response.getHeader("Content-Type"), "text/html");
	ASSERT_TRUE(response.getHeader("NotExist").empty());
	ASSERT_EQ(response.getBody(), "<html></html>");
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
				}));
		});
	}).then([server] {
		return server->listen({ HTTPD_LISTEN_IP, HTTPD_LISTEN_PORT });
	}).then([] {
		return seastar::do_with(
			seastar::semaphore(20),
			seastar::gate(),
			100UL,
			cpv::HttpClient::create("127.0.0.1", HTTPD_LISTEN_PORT),
			[] (auto& limit, auto& gate, auto& count, auto& client) {
			return seastar::repeat([&limit, &gate, &count, &client] {
				return seastar::get_units(limit, 1).then([&gate, &count, &client] (auto units) {
					gate.enter();
					client.makeRequest("GET", "/hello").send(client).then([] (auto response) {
						ASSERT_EQ(response.getStatus(), 200);
						ASSERT_EQ(response.getHeader("Server"), "Seastar httpd");
						ASSERT_EQ(response.getBody(), "\"hello world\"");
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

