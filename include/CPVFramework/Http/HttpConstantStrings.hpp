#pragma once
#include "../Utility/ConstantStrings.hpp"

namespace cpv::constants {
	// protocols
	extern const std::string Http10;
	extern const std::string Http11;
	extern const std::string Http12;
	
	// status codes
	extern const std::string _100;
	extern const std::string _101;
	extern const std::string _200;
	extern const std::string _201;
	extern const std::string _202;
	extern const std::string _203;
	extern const std::string _204;
	extern const std::string _205;
	extern const std::string _206;
	extern const std::string _300;
	extern const std::string _301;
	extern const std::string _302;
	extern const std::string _304;
	extern const std::string _305;
	extern const std::string _307;
	extern const std::string _400;
	extern const std::string _401;
	extern const std::string _402;
	extern const std::string _403;
	extern const std::string _404;
	extern const std::string _405;
	extern const std::string _406;
	extern const std::string _407;
	extern const std::string _408;
	extern const std::string _409;
	extern const std::string _410;
	extern const std::string _411;
	extern const std::string _412;
	extern const std::string _413;
	extern const std::string _414;
	extern const std::string _415;
	extern const std::string _416;
	extern const std::string _417;
	extern const std::string _500;
	extern const std::string _501;
	extern const std::string _502;
	extern const std::string _503;
	extern const std::string _504;
	extern const std::string _505;
	
	// status messages
	extern const std::string Continue; // 100
	extern const std::string SwitchingProtocols; // 101
	extern const std::string OK; // 200
	extern const std::string Created; // 201
	extern const std::string Accepted; // 202
	extern const std::string NonAuthoritativeInformation; // 203
	extern const std::string NoContent; // 204
	extern const std::string ResetContent; // 205
	extern const std::string PartialContent; // 206
	extern const std::string MultipleChoices; // 300
	extern const std::string MovedPermanently; // 301
	extern const std::string Found; // 302
	extern const std::string SeeOther; // 303
	extern const std::string NotModified; // 304
	extern const std::string UseProxy; // 305
	extern const std::string TemporaryRedirect; // 307
	extern const std::string BadRequest; // 400
	extern const std::string Unauthorized; // 401
	extern const std::string PaymentRequired; // 402
	extern const std::string Forbidden; // 403
	extern const std::string NotFound; // 404
	extern const std::string MethodNotAllowed; // 405
	extern const std::string NotAcceptable; // 406
	extern const std::string ProxyAuthenticationRequired; // 407
	extern const std::string RequestTimeout; // 408
	extern const std::string Conflict; // 409
	extern const std::string Gone; // 410
	extern const std::string LengthRequired; // 411
	extern const std::string PreconditionFailed; // 412
	extern const std::string RequstEntityTooLarge; // 413
	extern const std::string RequestURITooLong; // 414
	extern const std::string UnsupportedMediaType; // 415
	extern const std::string RequestedRangeNotSatisfiable; // 416
	extern const std::string ExpectationFailed; // 417
	extern const std::string InternalServerError; // 500
	extern const std::string NotImplemented; // 501
	extern const std::string BadGateway; // 502
	extern const std::string ServiceUnavailable; // 503
	extern const std::string GatewayTimeout; // 504
	extern const std::string HttpVersionNotSupported; // 505
	
	// standard response fields
	extern const std::string AccessControlAllowOrigin;
	extern const std::string AccessControlAllowCredentials;
	extern const std::string AccessControlExposeHeaders;
	extern const std::string AccessControlMaxAge;
	extern const std::string AccessControlAllowMethods;
	extern const std::string AccessControlAllowHeaders;
	extern const std::string AcceptPatch;
	extern const std::string AcceptRanges;
	extern const std::string Age;
	extern const std::string Allow;
	extern const std::string AltSvc;
	extern const std::string CacheControl;
	extern const std::string Connection;
	extern const std::string ContentDisposition;
	extern const std::string ContentEncoding;
	extern const std::string ContentLanguage;
	extern const std::string ContentLength;
	extern const std::string ContentLocation;
	extern const std::string ContentMD5;
	extern const std::string ContentRange;
	extern const std::string ContentType;
	extern const std::string Date;
	extern const std::string DeltaBase;
	extern const std::string Etag;
	extern const std::string Expires;
	extern const std::string IM;
	extern const std::string LastModified;
	extern const std::string Link;
	extern const std::string Location;
	extern const std::string P3P;
	extern const std::string Pragma;
	extern const std::string ProxyAuthenticate;
	extern const std::string PublicKeyPins;
	extern const std::string RetryAfter;
	extern const std::string Server;
	extern const std::string SetCookie;
	extern const std::string StrictTransportSecurity;
	extern const std::string Trailer;
	extern const std::string TransferEncoding;
	extern const std::string Tk;
	extern const std::string Upgrade;
	extern const std::string Vary;
	extern const std::string Via;
	extern const std::string Warning;
	extern const std::string WWWAuthenticate;
	extern const std::string XFrameOptions;
	
	// standard response values
	extern const std::string Chunked;
	extern const std::string Keepalive;
	extern const std::string Close;
	extern const std::string TextPlainUtf8;
	extern const std::string ApplicationJsonUtf8;
}

