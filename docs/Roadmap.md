# Roadmap

## 0.2

Done:

- (api change) add `SharedString` and use it instead of `std::string_view`, it make lifetime management much easier
- (api change) remove `getCacheControl` and `setCacheControl` from `HttpRequestHeaders`
- (api change) change paramter type from `const HttpServerRequestHandlerIterator&` to `HttpServerRequestHandlerIterator` in http request handler
- (api change) require types under `cpv::extensions::http_context_parameters` when registering http request handler with parameters
- add json serializer and deserializer

TODO:

- add static file handler
	- support pre-compressed gzip
	- support bytes range
	- support return 304 not modified when if-modified-since matched
- add form serializer and deserializer
- add template engine
- add prometheus module

## 0.3

- add http client
- add dynamic compress handler
	- support compress in once and compress in chunk
- add cql driver module (header only)
	- add crud example

## Backlog

- upgrade to c++20 for coroutine support
- add base class of model that provides id and underlying buffer member for convenient

