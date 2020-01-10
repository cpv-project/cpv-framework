# Release notes

## 0.2

- (api change) add `SharedString` and use it instead of `std::string_view`, it make lifetime management much easier
- (api change) remove `getCacheControl` and `setCacheControl` from `HttpRequestHeaders`
- (api change) change paramter type from `const HttpServerRequestHandlerIterator&` to `HttpServerRequestHandlerIterator` in http request handler
- (api change) require types under `cpv::extensions::http_context_parameters` when registering http request handler with parameters
- add json serializer and deserializer
- add form serializer and deserializer
- add static file handler
	- support pre-compressed gzip (detect original-filename.gz)
	- support bytes range
	- support return 304 not modified when If-Modified-Since matched
- add benchmark tools

## 0.1

- Add dependency injection container
- Add http server (supports http 1.0/1.1 and pipeline)
- Add application and module framework
	- Provide LoggingModule
	- Provide HttpServerModule
	- Provide HttpServerRoutingModule

