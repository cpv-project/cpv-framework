#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestOutputStreamExtensions, writeAll) {
	return seastar::do_with(
		cpv::StringOutputStream(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& stream, auto& target) {
		stream.reset(target);
		return cpv::extensions::writeAll(stream, std::string("test data")).then([&target] {
			ASSERT_EQ(*target, "test data");
		});
	});
}

TEST_FUTURE(TestOutputStreamExtensions, writeAll_ptr) {
	auto target = seastar::make_lw_shared<std::string>();
	return seastar::do_with(
		cpv::makeReusable<cpv::StringOutputStream>(target).cast<cpv::OutputStreamBase>(),
		cpv::Reusable<cpv::OutputStreamBase>(),
		target,
		[] (auto& stream, auto& nullStream, auto& target) {
		return cpv::extensions::writeAll(stream, std::string("test data")).then([&target] {
			ASSERT_EQ(*target, "test data");
		}).then([&nullStream] {
			return cpv::extensions::writeAll(nullStream, std::string(""));
		}).then_wrapped([] (auto&& f) {
			ASSERT_THROWS_CONTAINS(cpv::LogicException, f.get(), "write to null stream");
		});
	});
}

TEST_FUTURE(TestOutputStreamExtensions, writeAll_view_cstr) {
	return seastar::do_with(
		cpv::StringOutputStream(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& stream, auto& target) {
		stream.reset(target);
		return cpv::extensions::writeAll(stream, "test data").then([&target] {
			ASSERT_EQ(*target, "test data");
		});
	});
}

TEST_FUTURE(TestOutputStreamExtensions, writeAll_view_cstr_ptr) {
	auto target = seastar::make_lw_shared<std::string>();
	return seastar::do_with(
		cpv::makeReusable<cpv::StringOutputStream>(target).cast<cpv::OutputStreamBase>(),
		cpv::Reusable<cpv::OutputStreamBase>(),
		target,
		[] (auto& stream, auto& nullStream, auto& target) {
		return cpv::extensions::writeAll(stream, "test data").then([&target] {
			ASSERT_EQ(*target, "test data");
		}).then([&nullStream] {
			return cpv::extensions::writeAll(nullStream, "");
		}).then_wrapped([] (auto&& f) {
			ASSERT_THROWS_CONTAINS(cpv::LogicException, f.get(), "write to null stream");
		});
	});
}

