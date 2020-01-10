# Roadmap

## 0.3

- add http client
- add ssl support
	- support certbot
- add dynamic compress handler
	- support compress in once and compress in chunk

## 0.4

- add template engine
- add multipart form serializer and deserializer
- add cql driver module (header only)
	- add crud example

## Backlog

- upgrade to c++20 for coroutine support
- add prometheus module (big job because seastar's impl only works for seastar httpd)
- add base class of model that provides id and underlying buffer member for convenient

