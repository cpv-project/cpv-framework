#include <CPVFramework/Http/Client.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestHttpClient, buildRequest) {
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

