#include <iostream>
#include <seastar/core/app-template.hh>

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		return seastar::make_ready_future<>();
	});
	return 0;
}

