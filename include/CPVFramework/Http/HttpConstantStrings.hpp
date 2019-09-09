#pragma once
#include "../Utility/ConstantStrings.hpp"

namespace cpv::constants {
	// protocols
	static const constexpr std::string_view Http10("HTTP/1.0");
	static const constexpr std::string_view Http11("HTTP/1.1");
	static const constexpr std::string_view Http12("HTTP/1.2");
	
	// methods
	static const constexpr std::string_view GET("GET");
	static const constexpr std::string_view HEAD("HEAD");
	static const constexpr std::string_view POST("POST");
	static const constexpr std::string_view PUT("PUT");
	static const constexpr std::string_view DELETE("DELETE");
	static const constexpr std::string_view CONNECT("CONNECT");
	static const constexpr std::string_view OPTIONS("OPTIONS");
	static const constexpr std::string_view TRACE("TRACE");
	static const constexpr std::string_view PATCH("PATCH");

	// status codes
	static const constexpr std::string_view _100("100");
	static const constexpr std::string_view _101("101");
	static const constexpr std::string_view _200("200");
	static const constexpr std::string_view _201("201");
	static const constexpr std::string_view _202("202");
	static const constexpr std::string_view _203("203");
	static const constexpr std::string_view _204("204");
	static const constexpr std::string_view _205("205");
	static const constexpr std::string_view _206("206");
	static const constexpr std::string_view _300("300");
	static const constexpr std::string_view _301("301");
	static const constexpr std::string_view _302("302");
	static const constexpr std::string_view _304("304");
	static const constexpr std::string_view _305("305");
	static const constexpr std::string_view _307("307");
	static const constexpr std::string_view _400("400");
	static const constexpr std::string_view _401("401");
	static const constexpr std::string_view _402("402");
	static const constexpr std::string_view _403("403");
	static const constexpr std::string_view _404("404");
	static const constexpr std::string_view _405("405");
	static const constexpr std::string_view _406("406");
	static const constexpr std::string_view _407("407");
	static const constexpr std::string_view _408("408");
	static const constexpr std::string_view _409("409");
	static const constexpr std::string_view _410("410");
	static const constexpr std::string_view _411("411");
	static const constexpr std::string_view _412("412");
	static const constexpr std::string_view _413("413");
	static const constexpr std::string_view _414("414");
	static const constexpr std::string_view _415("415");
	static const constexpr std::string_view _416("416");
	static const constexpr std::string_view _417("417");
	static const constexpr std::string_view _500("500");
	static const constexpr std::string_view _501("501");
	static const constexpr std::string_view _502("502");
	static const constexpr std::string_view _503("503");
	static const constexpr std::string_view _504("504");
	static const constexpr std::string_view _505("505");
	
	// status messages
	static const constexpr std::string_view Continue("Continue");
	static const constexpr std::string_view SwitchingProtocols("Switching Protocols");
	static const constexpr std::string_view OK("OK");
	static const constexpr std::string_view Created("Created");
	static const constexpr std::string_view Accepted("Accepted");
	static const constexpr std::string_view NonAuthoritativeInformation("Non-Authoritative Information");
	static const constexpr std::string_view NoContent("No Content");
	static const constexpr std::string_view ResetContent("Reset Content");
	static const constexpr std::string_view PartialContent("Partial Content");
	static const constexpr std::string_view MultipleChoices("Multiple Choices");
	static const constexpr std::string_view MovedPermanently("Moved Permanently");
	static const constexpr std::string_view Found("Found");
	static const constexpr std::string_view SeeOther("See Other");
	static const constexpr std::string_view NotModified("Not Modified");
	static const constexpr std::string_view UseProxy("Use Proxy");
	static const constexpr std::string_view TemporaryRedirect("Temporary Redirect");
	static const constexpr std::string_view BadRequest("Bad Request");
	static const constexpr std::string_view Unauthorized("Unauthorized");
	static const constexpr std::string_view PaymentRequired("Payment Required");
	static const constexpr std::string_view Forbidden("Forbidden");
	static const constexpr std::string_view NotFound("Not Found");
	static const constexpr std::string_view MethodNotAllowed("Method Not Allowed");
	static const constexpr std::string_view NotAcceptable("Not Acceptable");
	static const constexpr std::string_view ProxyAuthenticationRequired("Proxy Authentication Required");
	static const constexpr std::string_view RequestTimeout("Request Timeout");
	static const constexpr std::string_view Conflict("Conflict");
	static const constexpr std::string_view Gone("Gone");
	static const constexpr std::string_view LengthRequired("Length Required");
	static const constexpr std::string_view PreconditionFailed("Precondition Failed");
	static const constexpr std::string_view RequstEntityTooLarge("Requst Entity Too Large");
	static const constexpr std::string_view RequestURITooLong("Request-URI Too Long");
	static const constexpr std::string_view UnsupportedMediaType("Unsupported Media Type");
	static const constexpr std::string_view RequestedRangeNotSatisfiable("Requested Range Not Satisfiable");
	static const constexpr std::string_view ExpectationFailed("Expectation Failed");
	static const constexpr std::string_view InternalServerError("Internal Server Error");
	static const constexpr std::string_view NotImplemented("Not Implemented");
	static const constexpr std::string_view BadGateway("Bad Gateway");
	static const constexpr std::string_view ServiceUnavailable("Service Unavailable");
	static const constexpr std::string_view GatewayTimeout("Gateway Timeout");
	static const constexpr std::string_view HttpVersionNotSupported("HTTP Version Not Supported");
	
	// standard request fields
	static const constexpr std::string_view AIM("A-IM");
	static const constexpr std::string_view Accept("Accept");
	static const constexpr std::string_view AcceptCharset("Accept-Charset");
	static const constexpr std::string_view AcceptDatetime("Accept-Datetime");
	static const constexpr std::string_view AcceptEncoding("Accept-Encoding");
	static const constexpr std::string_view AcceptLanguage("Accept-Language");
	static const constexpr std::string_view AccessControlRequestMethod("Access-Control-Request-Method");
	static const constexpr std::string_view AccessControlRequestHeaders("Access-Control-Request-Headers");
	static const constexpr std::string_view Authorization("Authorization");
	static const constexpr std::string_view Cookie("Cookie");
	static const constexpr std::string_view Expect("Expect");
	static const constexpr std::string_view Forwarded("Forwarded");
	static const constexpr std::string_view From("From");
	static const constexpr std::string_view Host("Host");
	static const constexpr std::string_view Http2Settings("HTTP2-Settings");
	static const constexpr std::string_view IfMatch("If-Match");
	static const constexpr std::string_view IfModifiedSince("If-Modified-Since");
	static const constexpr std::string_view IfNoneMatch("If-None-Match");
	static const constexpr std::string_view IfRange("If-Range");
	static const constexpr std::string_view IfUnmodifiedSince("If-Unmodified-Since");
	static const constexpr std::string_view MaxForwards("Max-Forwards");
	static const constexpr std::string_view Origin("Origin");
	static const constexpr std::string_view ProxyAuthorization("Proxy-Authorization");
	static const constexpr std::string_view Range("Range");
	static const constexpr std::string_view Referer("Referer");
	static const constexpr std::string_view TE("TE");
	static const constexpr std::string_view UserAgent("User-Agent");
	
	// non-standard request fields
	static const constexpr std::string_view UpgradeInsecureRequests("Upgrade-Insecure-Requests");
	static const constexpr std::string_view XRequestedWith("X-Requested-With");
	static const constexpr std::string_view DNT("DNT");
	static const constexpr std::string_view XCsrfToken("X-Csrf-Token");
	
	// standard response fields
	static const constexpr std::string_view AccessControlAllowOrigin("Access-Control-Allow-Origin");
	static const constexpr std::string_view AccessControlAllowCredentials("Access-Control-Allow-Credentials");
	static const constexpr std::string_view AccessControlExposeHeaders("Access-Control-Expose-Headers");
	static const constexpr std::string_view AccessControlMaxAge("Access-Control-Max-Age");
	static const constexpr std::string_view AccessControlAllowMethods("Access-Control-Allow-Methods");
	static const constexpr std::string_view AccessControlAllowHeaders("Access-Control-Allow-Headers");
	static const constexpr std::string_view AcceptPatch("Accept-Patch");
	static const constexpr std::string_view AcceptRanges("Accept-Ranges");
	static const constexpr std::string_view Age("Age");
	static const constexpr std::string_view Allow("Allow");
	static const constexpr std::string_view AltSvc("Alt-Svc");
	static const constexpr std::string_view CacheControl("Cache-Control");
	static const constexpr std::string_view Connection("Connection");
	static const constexpr std::string_view ContentDisposition("Content-Disposition");
	static const constexpr std::string_view ContentEncoding("Content-Encoding");
	static const constexpr std::string_view ContentLanguage("Content-Language");
	static const constexpr std::string_view ContentLength("Content-Length");
	static const constexpr std::string_view ContentLocation("Content-Location");
	static const constexpr std::string_view ContentMD5("Content-MD5");
	static const constexpr std::string_view ContentRange("Content-Range");
	static const constexpr std::string_view ContentType("Content-Type");
	static const constexpr std::string_view Date("Date");
	static const constexpr std::string_view DeltaBase("Delta-Base");
	static const constexpr std::string_view ETag("ETag");
	static const constexpr std::string_view Expires("Expires");
	static const constexpr std::string_view IM("IM");
	static const constexpr std::string_view LastModified("Last-Modified");
	static const constexpr std::string_view Link("Link");
	static const constexpr std::string_view Location("Location");
	static const constexpr std::string_view P3P("P3P");
	static const constexpr std::string_view Pragma("Pragma");
	static const constexpr std::string_view ProxyAuthenticate("Proxy-Authenticate");
	static const constexpr std::string_view PublicKeyPins("Public-Key-Pins");
	static const constexpr std::string_view RetryAfter("Retry-After");
	static const constexpr std::string_view Server("Server");
	static const constexpr std::string_view SetCookie("SetCookie");
	static const constexpr std::string_view StrictTransportSecurity("Strict-Transport-Security");
	static const constexpr std::string_view Trailer("Trailer");
	static const constexpr std::string_view TransferEncoding("Transfer-Encoding");
	static const constexpr std::string_view Tk("Tk");
	static const constexpr std::string_view Upgrade("Upgrade");
	static const constexpr std::string_view Vary("Vary");
	static const constexpr std::string_view Via("Via");
	static const constexpr std::string_view Warning("Warning");
	static const constexpr std::string_view WWWAuthenticate("WWW-Authenticate");
	static const constexpr std::string_view XFrameOptions("X-Frame-Options");
	
	// non-standard response fields
	static const constexpr std::string_view Refresh("Refresh");
	
	// standard response values
	static const constexpr std::string_view Chunked("chunked");
	static const constexpr std::string_view Keepalive("keep-alive");
	static const constexpr std::string_view Close("close");
	static const constexpr std::string_view TextPlainUtf8("text/plain;charset=utf-8");
	static const constexpr std::string_view ApplicationJsonUtf8("application/json;charset=utf-8");
}

