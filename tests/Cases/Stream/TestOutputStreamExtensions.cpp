#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Object.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestOutputStreamExtensions, writeAll) {
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

TEST_FUTURE(TestOutputStreamExtensions, writeAll_ptr) {
	auto target = seastar::make_lw_shared<std::string>();
	return seastar::do_with(
		cpv::makeObject<cpv::StringOutputStream>(target).cast<cpv::OutputStreamBase>(),
		cpv::Object<cpv::OutputStreamBase>(),
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

