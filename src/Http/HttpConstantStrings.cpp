#include <CPVFramework/Http/HttpConstantStrings.hpp>

namespace cpv::constants {
	// protocols
	const std::string Http10("HTTP/1.0");
	const std::string Http11("HTTP/1.1");
	const std::string Http12("HTTP/1.2");
	
	// methods
	const std::string GET("GET");
	const std::string HEAD("HEAD");
	const std::string POST("POST");
	const std::string PUT("PUT");
	const std::string DELETE("DELETE");
	const std::string CONNECT("CONNECT");
	const std::string OPTIONS("OPTIONS");
	const std::string TRACE("TRACE");
	const std::string PATCH("PATCH");

	// status codes
	const std::string _100("100");
	const std::string _101("101");
	const std::string _200("200");
	const std::string _201("201");
	const std::string _202("202");
	const std::string _203("203");
	const std::string _204("204");
	const std::string _205("205");
	const std::string _206("206");
	const std::string _300("300");
	const std::string _301("301");
	const std::string _302("302");
	const std::string _304("304");
	const std::string _305("305");
	const std::string _307("307");
	const std::string _400("400");
	const std::string _401("401");
	const std::string _402("402");
	const std::string _403("403");
	const std::string _404("404");
	const std::string _405("405");
	const std::string _406("406");
	const std::string _407("407");
	const std::string _408("408");
	const std::string _409("409");
	const std::string _410("410");
	const std::string _411("411");
	const std::string _412("412");
	const std::string _413("413");
	const std::string _414("414");
	const std::string _415("415");
	const std::string _416("416");
	const std::string _417("417");
	const std::string _500("500");
	const std::string _501("501");
	const std::string _502("502");
	const std::string _503("503");
	const std::string _504("504");
	const std::string _505("505");
	
	// status messages
	const std::string Continue("Continue");
	const std::string SwitchingProtocols("Switching Protocols");
	const std::string OK("OK");
	const std::string Created("Created");
	const std::string Accepted("Accepted");
	const std::string NonAuthoritativeInformation("Non-Authoritative Information");
	const std::string NoContent("No Content");
	const std::string ResetContent("Reset Content");
	const std::string PartialContent("Partial Content");
	const std::string MultipleChoices("Multiple Choices");
	const std::string MovedPermanently("Moved Permanently");
	const std::string Found("Found");
	const std::string SeeOther("See Other");
	const std::string NotModified("Not Modified");
	const std::string UseProxy("Use Proxy");
	const std::string TemporaryRedirect("Temporary Redirect");
	const std::string BadRequest("Bad Request");
	const std::string Unauthorized("Unauthorized");
	const std::string PaymentRequired("Payment Required");
	const std::string Forbidden("Forbidden");
	const std::string NotFound("Not Found");
	const std::string MethodNotAllowed("Method Not Allowed");
	const std::string NotAcceptable("Not Acceptable");
	const std::string ProxyAuthenticationRequired("Proxy Authentication Required");
	const std::string RequestTimeout("Request Timeout");
	const std::string Conflict("Conflict");
	const std::string Gone("Gone");
	const std::string LengthRequired("Length Required");
	const std::string PreconditionFailed("Precondition Failed");
	const std::string RequstEntityTooLarge("Requst Entity Too Large");
	const std::string RequestURITooLong("Request-URI Too Long");
	const std::string UnsupportedMediaType("Unsupported Media Type");
	const std::string RequestedRangeNotSatisfiable("Requested Range Not Satisfiable");
	const std::string ExpectationFailed("Expectation Failed");
	const std::string InternalServerError("Internal Server Error");
	const std::string NotImplemented("Not Implemented");
	const std::string BadGateway("Bad Gateway");
	const std::string ServiceUnavailable("Service Unavailable");
	const std::string GatewayTimeout("Gateway Timeout");
	const std::string HttpVersionNotSupported("HTTP Version Not Supported");
	
	// standard request fields
	const std::string AIM("A-IM");
	const std::string Accept("Accept");
	const std::string AcceptCharset("Accept-Charset");
	const std::string AcceptDatetime("Accept-Datetime");
	const std::string AcceptEncoding("Accept-Encoding");
	const std::string AcceptLanguage("Accept-Language");
	const std::string AccessControlRequestMethod("Access-Control-Request-Method");
	const std::string AccessControlRequestHeaders("Access-Control-Request-Headers");
	const std::string Authorization("Authorization");
	const std::string Cookie("Cookie");
	const std::string Expect("Expect");
	const std::string Forwarded("Forwarded");
	const std::string From("From");
	const std::string Host("Host");
	const std::string Http2Settings("HTTP2-Settings");
	const std::string IfMatch("If-Match");
	const std::string IfModifiedSince("If-Modified-Since");
	const std::string IfNoneMatch("If-None-Match");
	const std::string IfRange("If-Range");
	const std::string IfUnmodifiedSince("If-Unmodified-Since");
	const std::string MaxForwards("Max-Forwards");
	const std::string Origin("Origin");
	const std::string ProxyAuthorization("Proxy-Authorization");
	const std::string Range("Range");
	const std::string Referer("Referer");
	const std::string TE("TE");
	const std::string UserAgent("User-Agent");
	
	// non-standard request fields
	const std::string UpgradeInsecureRequests("Upgrade-Insecure-Requests");
	const std::string XRequestedWith("X-Requested-With");
	const std::string DNT("DNT");
	const std::string XCsrfToken("X-Csrf-Token");
	
	// standard response fields
	const std::string AccessControlAllowOrigin("Access-Control-Allow-Origin");
	const std::string AccessControlAllowCredentials("Access-Control-Allow-Credentials");
	const std::string AccessControlExposeHeaders("Access-Control-Expose-Headers");
	const std::string AccessControlMaxAge("Access-Control-Max-Age");
	const std::string AccessControlAllowMethods("Access-Control-Allow-Methods");
	const std::string AccessControlAllowHeaders("Access-Control-Allow-Headers");
	const std::string AcceptPatch("Accept-Patch");
	const std::string AcceptRanges("Accept-Ranges");
	const std::string Age("Age");
	const std::string Allow("Allow");
	const std::string AltSvc("Alt-Svc");
	const std::string CacheControl("Cache-Control");
	const std::string Connection("Connection");
	const std::string ContentDisposition("Content-Disposition");
	const std::string ContentEncoding("Content-Encoding");
	const std::string ContentLanguage("Content-Language");
	const std::string ContentLength("Content-Length");
	const std::string ContentLocation("Content-Location");
	const std::string ContentMD5("Content-MD5");
	const std::string ContentRange("Content-Range");
	const std::string ContentType("Content-Type");
	const std::string Date("Date");
	const std::string DeltaBase("Delta-Base");
	const std::string ETag("ETag");
	const std::string Expires("Expires");
	const std::string IM("IM");
	const std::string LastModified("Last-Modified");
	const std::string Link("Link");
	const std::string Location("Location");
	const std::string P3P("P3P");
	const std::string Pragma("Pragma");
	const std::string ProxyAuthenticate("Proxy-Authenticate");
	const std::string PublicKeyPins("Public-Key-Pins");
	const std::string RetryAfter("Retry-After");
	const std::string Server("Server");
	const std::string SetCookie("SetCookie");
	const std::string StrictTransportSecurity("Strict-Transport-Security");
	const std::string Trailer("Trailer");
	const std::string TransferEncoding("Transfer-Encoding");
	const std::string Tk("Tk");
	const std::string Upgrade("Upgrade");
	const std::string Vary("Vary");
	const std::string Via("Via");
	const std::string Warning("Warning");
	const std::string WWWAuthenticate("WWW-Authenticate");
	const std::string XFrameOptions("X-Frame-Options");
	
	// non-standard response fields
	const std::string Refresh("Refresh");
	
	// standard response values
	const std::string Chunked("chunked");
	const std::string Keepalive("keep-alive");
	const std::string Close("close");
	const std::string TextPlainUtf8("text/plain;charset=utf-8");
	const std::string ApplicationJsonUtf8("application/json;charset=utf-8");
}

