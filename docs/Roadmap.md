# Roadmap

## 0.2

Done:

- add json serializer and deserializer

TODO:

- add and use SharedString everywhere
- add static file handler
	- support pre-compressed gzip
	- support bytes range
	- support 304 not modified when if-modified-since matched
- add dynamic compress handler
- add template engine
- add form serializer and deserializer
- add prometheus module

## 0.3

- add http client
- add cql driver module (header only)
	- add crud example

## Backlog

- upgrade to c++20 for coroutine support
- add base class of model that provides id and underlying buffer member for convenient

