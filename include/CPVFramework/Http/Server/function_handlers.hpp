#pragma once
#include <http/function_handlers.hh>

namespace cpv::httpd {
	using seastar::httpd::request_function;
	using seastar::httpd::handle_function;
	using seastar::httpd::json_request_function;
	using seastar::httpd::future_json_function;
	using seastar::httpd::future_handler_function;
	using seastar::httpd::function_handler;
}

