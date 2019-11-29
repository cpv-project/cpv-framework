#include <cstdlib>
#include <boost/range/irange.hpp>
#include <seastar/core/future-util.hh>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestStaticFileHandler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	/** Prepare files for testing and cleanup after finished */
	class FileSetupFixture {
	public:
		FileSetupFixture() {
			::system(
				"mkdir -p /tmp/cpv-framework-static-file-handler-test && "
				"cd /tmp/cpv-framework-static-file-handler-test && "
				"echo -n abcde > simple=.txt && "
				"touch -m --date=\"Fri Nov 29 21:01:01 UTC 2019\" simple=.txt && "
				"mkdir -p child && cd child && "
				"echo -n qwertzxcvbasdfg > compress.txt && "
				"echo -n 123 > compress.txt.gz && "
				"touch -m --date=\"Fri Nov 29 21:02:02 UTC 2019\" compress.txt && "
				"touch -m --date=\"Fri Nov 29 21:03:03 UTC 2019\" compress.txt.gz"
			);
		}

		~FileSetupFixture() {
			::system("rm -rf /tmp/cpv-framework-static-file-handler-test");
		}
	};

	void prepareHandlers(
		cpv::HttpServerRequestHandlerCollection& handlers) {
		handlers.clear();
		handlers.emplace_back(
			seastar::make_shared<cpv::HttpServerRequestStaticFileHandler>(
			"/static", "/tmp/cpv-framework-static-file-handler-test", "max-age=84600, public"));
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest404Handler>());
	}

	void prepareContext(
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str,
		cpv::SharedString&& url) {
		str->clear();
		context.setRequestResponse(cpv::HttpRequest(), cpv::HttpResponse());
		context.getResponse().setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		context.getRequest().setUrl(std::move(url));
	}

	seastar::future<> testSimpleTxt(
		cpv::HttpServerRequestHandlerCollection& handlers,
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str) {
		static const boost::integer_range<int> range(0, 2);
		prepareHandlers(handlers);
		return seastar::do_for_each(range, [&handlers, &context, &str] (auto i) {
			// first time is from disk, second time is from cache
			prepareContext(context, str, "/static/simple%3d.txt");
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&context, &str, i] {
				auto& response = context.getResponse();
				ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
				ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
				auto& headers = response.getHeaders();
				ASSERT_EQ(headers.getContentType(), "text/plain");
				ASSERT_EQ(headers.getContentLength(), "5");
				ASSERT_EQ(headers.getCacheControl(), "max-age=84600, public");
				ASSERT_EQ(headers.getContentEncoding(), "");
				ASSERT_EQ(headers.getLastModified(), "Fri, 29 Nov 2019 21:01:01 GMT");
				if (i == 0) {
					ASSERT_EQ(headers.getHeader("X-Cache"), "");
				} else {
					ASSERT_EQ(headers.getHeader("X-Cache"), "HIT");
				}
				ASSERT_EQ(str->view(), "abcde");
			});
		});
	}

	seastar::future<> testCompressTxt(
		cpv::HttpServerRequestHandlerCollection& handlers,
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str) {
		static const boost::integer_range<int> range(0, 3);
		prepareHandlers(handlers);
		return seastar::do_for_each(range, [&handlers, &context, &str] (auto i) {
			// first time not accept compressed format, second time accept
			// third time accept and let it return from cache
			prepareContext(context, str, "/static/child/compress.txt");
			if (i != 0) {
				auto& headers = context.getRequest().getHeaders();
				headers.setAcceptEncoding("gzip, deflate");
			}
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&context, &str, i] {
				auto& response = context.getResponse();
				ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
				ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
				auto& headers = response.getHeaders();
				ASSERT_EQ(headers.getContentType(), "text/plain");
				ASSERT_EQ(headers.getCacheControl(), "max-age=84600, public");
				if (i == 0) {
					ASSERT_EQ(headers.getHeader("X-Cache"), "");
					ASSERT_EQ(headers.getContentEncoding(), "");
					ASSERT_EQ(headers.getContentLength(), "15");
					ASSERT_EQ(headers.getLastModified(), "Fri, 29 Nov 2019 21:02:02 GMT");
					ASSERT_EQ(str->view(), "qwertzxcvbasdfg");
				} else if (i == 1) {
					ASSERT_EQ(headers.getHeader("X-Cache"), "");
					ASSERT_EQ(headers.getContentEncoding(), "gzip");
					ASSERT_EQ(headers.getContentLength(), "3");
					ASSERT_EQ(headers.getLastModified(), "Fri, 29 Nov 2019 21:03:03 GMT");
					ASSERT_EQ(str->view(), "123");
				} else {
					ASSERT_EQ(headers.getHeader("X-Cache"), "HIT");
					ASSERT_EQ(headers.getContentEncoding(), "gzip");
					ASSERT_EQ(headers.getContentLength(), "3");
					ASSERT_EQ(headers.getLastModified(), "Fri, 29 Nov 2019 21:03:03 GMT");
					ASSERT_EQ(str->view(), "123");
				}
			});
		});
	}

	seastar::future<> test404NotFound(
		cpv::HttpServerRequestHandlerCollection& handlers,
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str) {
		prepareHandlers(handlers);
		prepareContext(context, str, "/static/simple1.txt");
		return handlers.at(0)->handle(context, handlers.begin() + 1).then([&context, &str] {
			auto& response = context.getResponse();
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_404);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::NotFound);
		});
	}

	seastar::future<> test302NotModified(
		cpv::HttpServerRequestHandlerCollection& handlers,
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str) {
		static const boost::integer_range<int> range(0, 3);
		prepareHandlers(handlers);
		return seastar::do_for_each(range, [&handlers, &context, &str] (auto i) {
			// first time is from disk, second time is from disk and set cache
			// third time is from cache
			prepareContext(context, str, "/static/simple%3d.txt");
			if (i != 1) {
				auto& headers = context.getRequest().getHeaders();
				headers.setHeader(cpv::constants::IfModifiedSince, "Fri, 29 Nov 2019 21:01:01 GMT");
			}
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&context, &str, i] {
				auto& response = context.getResponse();
				if (i == 1) {
					ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
					ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
					return;
				}
				ASSERT_EQ(response.getStatusCode(), cpv::constants::_304);
				ASSERT_EQ(response.getStatusMessage(), cpv::constants::NotModified);
				auto& headers = response.getHeaders();
				ASSERT_EQ(headers.getContentType(), "text/plain");
				ASSERT_EQ(headers.getContentLength(), "");
				ASSERT_EQ(headers.getCacheControl(), "max-age=84600, public");
				ASSERT_EQ(headers.getContentEncoding(), "");
				ASSERT_EQ(headers.getLastModified(), "Fri, 29 Nov 2019 21:01:01 GMT");
				if (i == 0) {
					ASSERT_EQ(headers.getHeader("X-Cache"), "");
				} else {
					ASSERT_EQ(headers.getHeader("X-Cache"), "HIT");
				}
			});
		});
	}

	seastar::future<> testFullRange(
		cpv::HttpServerRequestHandlerCollection& handlers,
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str) {
		static const boost::integer_range<int> range(0, 2);
		prepareHandlers(handlers);
		return seastar::do_for_each(range, [&handlers, &context, &str] (auto i) {
			// second time is testing whether the handler could bypass cache and
			// compressed file when range header present
			prepareContext(context, str, "/static/child/compress.txt");
			if (i == 1) {
				auto& headers = context.getRequest().getHeaders();
				headers.setHeader(cpv::constants::Range, "bytes=1-12");
			}
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&context, &str, i] {
				auto& response = context.getResponse();
				if (i == 0) {
					ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
					ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
					return;
				}
				ASSERT_EQ(response.getStatusCode(), cpv::constants::_206);
				ASSERT_EQ(response.getStatusMessage(), cpv::constants::PartialContent);
				auto& headers = response.getHeaders();
				ASSERT_EQ(headers.getContentType(), "text/plain");
				ASSERT_EQ(headers.getContentLength(), "12");
				ASSERT_EQ(headers.getCacheControl(), "max-age=84600, public");
				ASSERT_EQ(headers.getContentEncoding(), "");
				ASSERT_EQ(headers.getLastModified(), "");
				ASSERT_EQ(headers.getHeader(cpv::constants::ContentRange), "bytes 1-12/15");
				ASSERT_EQ(headers.getHeader("X-Cache"), "");
				ASSERT_EQ(str->view(), "wertzxcvbasd");
			});
		});
		return seastar::make_ready_future<>();
	}

	seastar::future<> testHalfRange(
		cpv::HttpServerRequestHandlerCollection& handlers,
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str) {
		static const boost::integer_range<int> range(0, 2);
		prepareHandlers(handlers);
		return seastar::do_for_each(range, [&handlers, &context, &str] (auto i) {
			// second time is testing whether the handler could bypass cache and
			// compressed file when range header present
			prepareContext(context, str, "/static/child/compress.txt");
			if (i == 1) {
				auto& headers = context.getRequest().getHeaders();
				headers.setHeader(cpv::constants::Range, "bytes=2-");
			}
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&context, &str, i] {
				auto& response = context.getResponse();
				if (i == 0) {
					ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
					ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
					return;
				}
				ASSERT_EQ(response.getStatusCode(), cpv::constants::_206);
				ASSERT_EQ(response.getStatusMessage(), cpv::constants::PartialContent);
				auto& headers = response.getHeaders();
				ASSERT_EQ(headers.getContentType(), "text/plain");
				ASSERT_EQ(headers.getContentLength(), "13");
				ASSERT_EQ(headers.getCacheControl(), "max-age=84600, public");
				ASSERT_EQ(headers.getContentEncoding(), "");
				ASSERT_EQ(headers.getLastModified(), "");
				ASSERT_EQ(headers.getHeader("X-Cache"), "");
				ASSERT_EQ(headers.getHeader(cpv::constants::ContentRange), "bytes 2-14/15");
				ASSERT_EQ(str->view(), "ertzxcvbasdfg");
			});
		});
		return seastar::make_ready_future<>();
	}

	seastar::future<> testInvalidRange(
		cpv::HttpServerRequestHandlerCollection& handlers,
		cpv::HttpContext& context,
		seastar::lw_shared_ptr<cpv::SharedStringBuilder>& str) {
		static const boost::integer_range<int> range(0, 6);
		prepareHandlers(handlers);
		return seastar::do_for_each(range, [&handlers, &context, &str] (auto i) {
			prepareContext(context, str, "/static/simple%3d.txt");
			auto& headers = context.getRequest().getHeaders();
			if (i == 0) {
				headers.setHeader(cpv::constants::Range, "chars=1-12");
			} else if (i == 1) {
				headers.setHeader(cpv::constants::Range, "bytes=100-");
			} else if (i == 2) {
				headers.setHeader(cpv::constants::Range, "bytes=12-1");
			} else if (i == 3) {
				headers.setHeader(cpv::constants::Range, "bytes=a-");
			} else if (i == 4) {
				headers.setHeader(cpv::constants::Range, "bytes=-a");
			} else if (i == 5) {
				headers.setHeader(cpv::constants::Range, "bytes=");
			}
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&context, &str, i] {
				auto& response = context.getResponse();
				ASSERT_EQ(response.getStatusCode(), cpv::constants::_200);
				ASSERT_EQ(response.getStatusMessage(), cpv::constants::OK);
				auto& headers = response.getHeaders();
				ASSERT_EQ(headers.getContentType(), "text/plain");
				ASSERT_EQ(headers.getContentLength(), "5");
				ASSERT_EQ(headers.getCacheControl(), "max-age=84600, public");
				ASSERT_EQ(headers.getContentEncoding(), "");
				ASSERT_EQ(headers.getLastModified(), "Fri, 29 Nov 2019 21:01:01 GMT");
				ASSERT_EQ(headers.getHeader("X-Cache"), "");
				ASSERT_EQ(str->view(), "abcde");
			});
		});
	}
}

TEST_FUTURE(HttpServerRequestStaticFileHandler, handle) {
	return seastar::do_with(
		std::make_unique<FileSetupFixture>(),
		cpv::HttpServerRequestHandlerCollection(),
		cpv::HttpContext(),
		seastar::make_lw_shared<cpv::SharedStringBuilder>(),
		[] (auto&, auto& handlers, auto& context, auto& str) {
		return testSimpleTxt(handlers, context, str).then([&handlers, &context, &str] {
			return testCompressTxt(handlers, context, str);
		}).then([&handlers, &context, &str] {
			return test404NotFound(handlers, context, str);
		}).then([&handlers, &context, &str] {
			return test302NotModified(handlers, context, str);
		}).then([&handlers, &context, &str] {
			return testFullRange(handlers, context, str);
		}).then([&handlers, &context, &str] {
			return testHalfRange(handlers, context, str);
		}).then([&handlers, &context, &str] {
			return testInvalidRange(handlers, context, str);
		});
	});
}

