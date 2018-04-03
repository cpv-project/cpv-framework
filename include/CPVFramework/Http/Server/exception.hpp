#pragma once
#include <http/exception.hh>

namespace cpv::httpd {
	using seastar::httpd::base_exception;
	using seastar::httpd::redirect_exception;
	using seastar::httpd::not_found_exception;
	using seastar::httpd::bad_request_exception;
	using seastar::httpd::bad_param_exception;
	using seastar::httpd::missing_param_exception;
	using seastar::httpd::json_exception;
}

