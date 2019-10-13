# C++ web framework based on seastar framework

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/79809aeed9b146f4aa52e9247b5eaf2c)](https://www.codacy.com/app/compiv/cpv-framework?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=cpv-project/cpv-framework&amp;utm_campaign=Badge_Grade)
[![Build Status](https://travis-ci.org/cpv-project/cpv-framework.svg?branch=master)](https://travis-ci.org/cpv-project/cpv-framework)
[![license](https://img.shields.io/github/license/cpv-project/cpv-framework.svg)]() 
[![GitHub release](https://img.shields.io/github/release/cpv-project/cpv-framework.svg)]()

cpv framework is a web framework written in c++ based on [seastar framework](https://github.com/scylladb/seastar), it focus on high performance and modular design.

seastar framework is a application framework that use the share nothing programming model, which isolate resources explicitly for each cpu core. cpv framework use this model too, a cpv application will initialize services on each cpu core, execute them on each cpu core and avoid sharing data between cpu cores, you don't need thread locks or atomic variables in cpv application because all code will be thread safe by design.

In addition, cpv framework avoid memory copies by using `std::string_view` and scattered message (struct iovec) everywhere, you can get a string view of url path, query parameter, http header, and body content of request inside the original packet (there few cases require copy such as http header value splited in two packets, or encoded query parameter different from original), and you can construct a scattered message as http response body by using `cpv::Packet` (which uses struct iovec for multiple segments).

For more features, see the feature list and documents below.

## Features

- Isolate resources explicitly between cpu cores
- Avoid memory copies by using `std::string_view` and scattered messages everywhere
- Future promise based asynchronous interface
- Dependency injection container
- Modular design
	- Modules decide what to do when application start and stop
	- Modules register services to dependency injection container and retrive services from it
- Http server
	- Supports Http 1.0/1.1 (use [http-parser](https://github.com/nodejs/http-parser))
	- Supports pipeline
	- Supports chained multiple request handlers (middleware style)
	- Supports full and wildcard url routing (by using routing handler)
	- Provide stream interface for request body and response body

You can also check the [roadmap](./docs/Roadmap.md) to see which features will be added in the next release.

## Getting Started

### Install cpv framework

TODO

### Write a hello world application

TODO

### Compile and execute the hello world application

TODO

## Documents

- [Application and module](TODO)
- [Dependency injection container](TODO)
- [Http request handler](TODO)

## Running Tests

TODO

## Contribution

TODO

## License

LICENSE: MIT LICENSE<br/>
Copyright © 2018-2019 303248153@github<br/>
If you have any license issue please contact 303248153@qq.com.

