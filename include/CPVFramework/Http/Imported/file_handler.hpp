#pragma once
#include <http/file_handler.hh>

namespace cpv::httpd {
	using seastar::httpd::file_transformer;
	using seastar::httpd::file_interaction_handler;
	using seastar::httpd::directory_handler;
	using seastar::httpd::file_handler;
}

