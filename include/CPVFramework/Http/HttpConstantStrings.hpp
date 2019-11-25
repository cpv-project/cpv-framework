#pragma once
#include "../Utility/ConstantStrings.hpp"

namespace cpv::constants {
	// protocols
	static const constexpr char Http10[] = "HTTP/1.0";
	static const constexpr char Http11[] = "HTTP/1.1";
	static const constexpr char Http12[] = "HTTP/1.2";
	
	// methods
	static const constexpr char GET[] = "GET";
	static const constexpr char HEAD[] = "HEAD";
	static const constexpr char POST[] = "POST";
	static const constexpr char PUT[] = "PUT";
	static const constexpr char DELETE[] = "DELETE";
	static const constexpr char CONNECT[] = "CONNECT";
	static const constexpr char OPTIONS[] = "OPTIONS";
	static const constexpr char TRACE[] = "TRACE";
	static const constexpr char PATCH[] = "PATCH";

	// status codes
	static const constexpr char _100[] = "100";
	static const constexpr char _101[] = "101";
	static const constexpr char _200[] = "200";
	static const constexpr char _201[] = "201";
	static const constexpr char _202[] = "202";
	static const constexpr char _203[] = "203";
	static const constexpr char _204[] = "204";
	static const constexpr char _205[] = "205";
	static const constexpr char _206[] = "206";
	static const constexpr char _300[] = "300";
	static const constexpr char _301[] = "301";
	static const constexpr char _302[] = "302";
	static const constexpr char _304[] = "304";
	static const constexpr char _305[] = "305";
	static const constexpr char _307[] = "307";
	static const constexpr char _400[] = "400";
	static const constexpr char _401[] = "401";
	static const constexpr char _402[] = "402";
	static const constexpr char _403[] = "403";
	static const constexpr char _404[] = "404";
	static const constexpr char _405[] = "405";
	static const constexpr char _406[] = "406";
	static const constexpr char _407[] = "407";
	static const constexpr char _408[] = "408";
	static const constexpr char _409[] = "409";
	static const constexpr char _410[] = "410";
	static const constexpr char _411[] = "411";
	static const constexpr char _412[] = "412";
	static const constexpr char _413[] = "413";
	static const constexpr char _414[] = "414";
	static const constexpr char _415[] = "415";
	static const constexpr char _416[] = "416";
	static const constexpr char _417[] = "417";
	static const constexpr char _500[] = "500";
	static const constexpr char _501[] = "501";
	static const constexpr char _502[] = "502";
	static const constexpr char _503[] = "503";
	static const constexpr char _504[] = "504";
	static const constexpr char _505[] = "505";
	
	// status messages
	static const constexpr char Continue[] = "Continue";
	static const constexpr char SwitchingProtocols[] = "Switching Protocols";
	static const constexpr char OK[] = "OK";
	static const constexpr char Created[] = "Created";
	static const constexpr char Accepted[] = "Accepted";
	static const constexpr char NonAuthoritativeInformation[] = "Non-Authoritative Information";
	static const constexpr char NoContent[] = "No Content";
	static const constexpr char ResetContent[] = "Reset Content";
	static const constexpr char PartialContent[] = "Partial Content";
	static const constexpr char MultipleChoices[] = "Multiple Choices";
	static const constexpr char MovedPermanently[] = "Moved Permanently";
	static const constexpr char Found[] = "Found";
	static const constexpr char SeeOther[] = "See Other";
	static const constexpr char NotModified[] = "Not Modified";
	static const constexpr char UseProxy[] = "Use Proxy";
	static const constexpr char TemporaryRedirect[] = "Temporary Redirect";
	static const constexpr char BadRequest[] = "Bad Request";
	static const constexpr char Unauthorized[] = "Unauthorized";
	static const constexpr char PaymentRequired[] = "Payment Required";
	static const constexpr char Forbidden[] = "Forbidden";
	static const constexpr char NotFound[] = "Not Found";
	static const constexpr char MethodNotAllowed[] = "Method Not Allowed";
	static const constexpr char NotAcceptable[] = "Not Acceptable";
	static const constexpr char ProxyAuthenticationRequired[] = "Proxy Authentication Required";
	static const constexpr char RequestTimeout[] = "Request Timeout";
	static const constexpr char Conflict[] = "Conflict";
	static const constexpr char Gone[] = "Gone";
	static const constexpr char LengthRequired[] = "Length Required";
	static const constexpr char PreconditionFailed[] = "Precondition Failed";
	static const constexpr char RequstEntityTooLarge[] = "Requst Entity Too Large";
	static const constexpr char RequestURITooLong[] = "Request-URI Too Long";
	static const constexpr char UnsupportedMediaType[] = "Unsupported Media Type";
	static const constexpr char RequestedRangeNotSatisfiable[] = "Requested Range Not Satisfiable";
	static const constexpr char ExpectationFailed[] = "Expectation Failed";
	static const constexpr char InternalServerError[] = "Internal Server Error";
	static const constexpr char NotImplemented[] = "Not Implemented";
	static const constexpr char BadGateway[] = "Bad Gateway";
	static const constexpr char ServiceUnavailable[] = "Service Unavailable";
	static const constexpr char GatewayTimeout[] = "Gateway Timeout";
	static const constexpr char HttpVersionNotSupported[] = "HTTP Version Not Supported";
	
	// standard request fields
	static const constexpr char AIM[] = "A-IM";
	static const constexpr char Accept[] = "Accept";
	static const constexpr char AcceptCharset[] = "Accept-Charset";
	static const constexpr char AcceptDatetime[] = "Accept-Datetime";
	static const constexpr char AcceptEncoding[] = "Accept-Encoding";
	static const constexpr char AcceptLanguage[] = "Accept-Language";
	static const constexpr char AccessControlRequestMethod[] = "Access-Control-Request-Method";
	static const constexpr char AccessControlRequestHeaders[] = "Access-Control-Request-Headers";
	static const constexpr char Authorization[] = "Authorization";
	static const constexpr char Cookie[] = "Cookie";
	static const constexpr char Expect[] = "Expect";
	static const constexpr char Forwarded[] = "Forwarded";
	static const constexpr char From[] = "From";
	static const constexpr char Host[] = "Host";
	static const constexpr char Http2Settings[] = "HTTP2-Settings";
	static const constexpr char IfMatch[] = "If-Match";
	static const constexpr char IfModifiedSince[] = "If-Modified-Since";
	static const constexpr char IfNoneMatch[] = "If-None-Match";
	static const constexpr char IfRange[] = "If-Range";
	static const constexpr char IfUnmodifiedSince[] = "If-Unmodified-Since";
	static const constexpr char MaxForwards[] = "Max-Forwards";
	static const constexpr char Origin[] = "Origin";
	static const constexpr char ProxyAuthorization[] = "Proxy-Authorization";
	static const constexpr char Range[] = "Range";
	static const constexpr char Referer[] = "Referer";
	static const constexpr char TE[] = "TE";
	static const constexpr char UserAgent[] = "User-Agent";
	
	// non-standard request fields
	static const constexpr char UpgradeInsecureRequests[] = "Upgrade-Insecure-Requests";
	static const constexpr char XRequestedWith[] = "X-Requested-With";
	static const constexpr char DNT[] = "DNT";
	static const constexpr char XCsrfToken[] = "X-Csrf-Token";
	
	// standard response fields
	static const constexpr char AccessControlAllowOrigin[] = "Access-Control-Allow-Origin";
	static const constexpr char AccessControlAllowCredentials[] = "Access-Control-Allow-Credentials";
	static const constexpr char AccessControlExposeHeaders[] = "Access-Control-Expose-Headers";
	static const constexpr char AccessControlMaxAge[] = "Access-Control-Max-Age";
	static const constexpr char AccessControlAllowMethods[] = "Access-Control-Allow-Methods";
	static const constexpr char AccessControlAllowHeaders[] = "Access-Control-Allow-Headers";
	static const constexpr char AcceptPatch[] = "Accept-Patch";
	static const constexpr char AcceptRanges[] = "Accept-Ranges";
	static const constexpr char Age[] = "Age";
	static const constexpr char Allow[] = "Allow";
	static const constexpr char AltSvc[] = "Alt-Svc";
	static const constexpr char CacheControl[] = "Cache-Control";
	static const constexpr char Connection[] = "Connection";
	static const constexpr char ContentDisposition[] = "Content-Disposition";
	static const constexpr char ContentEncoding[] = "Content-Encoding";
	static const constexpr char ContentLanguage[] = "Content-Language";
	static const constexpr char ContentLength[] = "Content-Length";
	static const constexpr char ContentLocation[] = "Content-Location";
	static const constexpr char ContentMD5[] = "Content-MD5";
	static const constexpr char ContentRange[] = "Content-Range";
	static const constexpr char ContentType[] = "Content-Type";
	static const constexpr char Date[] = "Date";
	static const constexpr char DeltaBase[] = "Delta-Base";
	static const constexpr char ETag[] = "ETag";
	static const constexpr char Expires[] = "Expires";
	static const constexpr char IM[] = "IM";
	static const constexpr char LastModified[] = "Last-Modified";
	static const constexpr char Link[] = "Link";
	static const constexpr char Location[] = "Location";
	static const constexpr char P3P[] = "P3P";
	static const constexpr char Pragma[] = "Pragma";
	static const constexpr char ProxyAuthenticate[] = "Proxy-Authenticate";
	static const constexpr char PublicKeyPins[] = "Public-Key-Pins";
	static const constexpr char RetryAfter[] = "Retry-After";
	static const constexpr char Server[] = "Server";
	static const constexpr char SetCookie[] = "SetCookie";
	static const constexpr char StrictTransportSecurity[] = "Strict-Transport-Security";
	static const constexpr char Trailer[] = "Trailer";
	static const constexpr char TransferEncoding[] = "Transfer-Encoding";
	static const constexpr char Tk[] = "Tk";
	static const constexpr char Upgrade[] = "Upgrade";
	static const constexpr char Vary[] = "Vary";
	static const constexpr char Via[] = "Via";
	static const constexpr char Warning[] = "Warning";
	static const constexpr char WWWAuthenticate[] = "WWW-Authenticate";
	static const constexpr char XFrameOptions[] = "X-Frame-Options";
	
	// non-standard response fields
	static const constexpr char Refresh[] = "Refresh";
	
	// standard response values
	static const constexpr char Chunked[] = "chunked";
	static const constexpr char Keepalive[] = "keep-alive";
	static const constexpr char Close[] = "close";
	static const constexpr char TextPlainUtf8[] = "text/plain;charset=utf-8";
	static const constexpr char ApplicationJsonUtf8[] = "application/json;charset=utf-8";
}

