#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(OutputStreamExtensions, writeAll) {
	return seastar::do_with(
		cpv::StringOutputStream(),
		seastar::make_lw_shared<cpv::SharedStringBuilder>(),
		[] (auto& stream, auto& target) {
		stream.reset(target);
		return cpv::extensions::writeAll(stream, "test data").then([&target] {
			ASSERT_EQ(target->view(), "test data");
		});
	});
}

TEST_FUTURE(OutputStreamExtensions, writeAll_ptr) {
	auto target = seastar::make_lw_shared<cpv::SharedStringBuilder>("str:");
	return seastar::do_with(
		cpv::makeReusable<cpv::StringOutputStream>(target).cast<cpv::OutputStreamBase>(),
		cpv::Reusable<cpv::OutputStreamBase>(),
		target,
		[] (auto& stream, auto& nullStream, auto& target) {
		return cpv::extensions::writeAll(stream, "test data").then([&target] {
			ASSERT_EQ(target->view(), "str:test data");
		}).then([&nullStream] {
			return cpv::extensions::writeAll(nullStream, "");
		}).then_wrapped([] (seastar::future<> f) {
			ASSERT_THROWS_CONTAINS(cpv::LogicException, f.get(), "write to null stream");
		});
	});
}

