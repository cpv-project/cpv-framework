#include <CPVFramework/Utility/ObjectTrait.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	struct TestReusable {
		int value = 0;
		static void freeResources() { }
		void reset(int newValue) { value = newValue; }
	};
}

template <>
thread_local cpv::ReusableStorageType<TestReusable>
	cpv::ReusableStorageInstance<TestReusable>;

TEST(ObjectTrait, defaultImpl) {
	using Trait = cpv::ObjectTrait<int>;
	int value = Trait::create(123);
	ASSERT_EQ(value, 123);
	Trait::reset(value);
	ASSERT_EQ(value, 0);
	Trait::set(value, 123);
	ASSERT_EQ(value, 123);
	ASSERT_EQ(Trait::get(value), 123);
	ASSERT_EQ(Trait::get(std::as_const(value)), 123);
}

TEST(ObjectTrait, optional) {
	using Trait = cpv::ObjectTrait<std::optional<int>>;
	std::optional<int> value = Trait::create(123);
	ASSERT_TRUE(value.has_value());
	ASSERT_EQ(*value, 123);
	Trait::reset(value);
	ASSERT_FALSE(value.has_value());
	ASSERT_EQ(Trait::get(value), nullptr);
	ASSERT_EQ(Trait::get(std::as_const(value)), nullptr);
	Trait::set(value, 123);
	ASSERT_TRUE(value.has_value());
	ASSERT_EQ(*value, 123);
	ASSERT_NE(Trait::get(value), nullptr);
	ASSERT_NE(Trait::get(std::as_const(value)), nullptr);
	ASSERT_EQ(*Trait::get(value), 123);
	ASSERT_EQ(*Trait::get(std::as_const(value)), 123);
	Trait::set(value, 321);
	ASSERT_TRUE(value.has_value());
	ASSERT_EQ(*value, 321);
}

TEST(ObjectTrait, unique_ptr) {
	using Trait = cpv::ObjectTrait<std::unique_ptr<int>>;
	std::unique_ptr<int> value = Trait::create(123);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(*value, 123);
	Trait::reset(value);
	ASSERT_FALSE(value != nullptr);
	ASSERT_EQ(Trait::get(value), nullptr);
	ASSERT_EQ(Trait::get(std::as_const(value)), nullptr);
	Trait::set(value, 123);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(*value, 123);
	ASSERT_NE(Trait::get(value), nullptr);
	ASSERT_NE(Trait::get(std::as_const(value)), nullptr);
	ASSERT_EQ(*Trait::get(value), 123);
	ASSERT_EQ(*Trait::get(std::as_const(value)), 123);
	Trait::set(value, 321);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(*value, 321);
}

TEST(ObjectTrait, shared_ptr) {
	using Trait = cpv::ObjectTrait<seastar::shared_ptr<int>>;
	seastar::shared_ptr<int> value = Trait::create(123);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(*value, 123);
	Trait::reset(value);
	ASSERT_FALSE(value != nullptr);
	ASSERT_EQ(Trait::get(value), nullptr);
	ASSERT_EQ(Trait::get(std::as_const(value)), nullptr);
	Trait::set(value, 123);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(*value, 123);
	ASSERT_NE(Trait::get(value), nullptr);
	ASSERT_NE(Trait::get(std::as_const(value)), nullptr);
	ASSERT_EQ(*Trait::get(value), 123);
	ASSERT_EQ(*Trait::get(std::as_const(value)), 123);
	Trait::set(value, 321);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(*value, 321);
}

TEST(ObjectTrait, lw_shared_ptr) {
	using Trait = cpv::ObjectTrait<seastar::lw_shared_ptr<int>>;
	seastar::lw_shared_ptr<int> value = Trait::create(123);
	ASSERT_TRUE(value.get() != nullptr);
	ASSERT_EQ(*value, 123);
	Trait::reset(value);
	ASSERT_FALSE(value.get() != nullptr);
	ASSERT_EQ(Trait::get(value), nullptr);
	ASSERT_EQ(Trait::get(std::as_const(value)), nullptr);
	Trait::set(value, 123);
	ASSERT_TRUE(value.get() != nullptr);
	ASSERT_EQ(*value, 123);
	ASSERT_NE(Trait::get(value), nullptr);
	ASSERT_NE(Trait::get(std::as_const(value)), nullptr);
	ASSERT_EQ(*Trait::get(value), 123);
	ASSERT_EQ(*Trait::get(std::as_const(value)), 123);
	Trait::set(value, 321);
	ASSERT_TRUE(value.get() != nullptr);
	ASSERT_EQ(*value, 321);
}

TEST(ObjectTrait, reusable) {
	using Trait = cpv::ObjectTrait<cpv::Reusable<TestReusable>>;
	cpv::Reusable<TestReusable> value = Trait::create(123);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(value->value, 123);
	Trait::reset(value);
	ASSERT_FALSE(value != nullptr);
	ASSERT_EQ(Trait::get(value), nullptr);
	ASSERT_EQ(Trait::get(std::as_const(value)), nullptr);
	Trait::set(value, 123);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(value->value, 123);
	ASSERT_NE(Trait::get(value), nullptr);
	ASSERT_NE(Trait::get(std::as_const(value)), nullptr);
	ASSERT_EQ(Trait::get(value)->value, 123);
	ASSERT_EQ(Trait::get(std::as_const(value))->value, 123);
	Trait::set(value, 321);
	ASSERT_TRUE(value != nullptr);
	ASSERT_EQ(value->value, 321);
}

TEST(ObjectTrait, vector) {
	using Vector = std::vector<int>;
	using Trait = cpv::ObjectTrait<Vector>;
	Vector value = Trait::create(Vector({ 1, 2, 3 }));
	ASSERT_EQ(value.size(), 3U);
	Trait::reset(value);
	ASSERT_EQ(value.size(), 0U);
	Trait::set(value, Vector({ 1, 2 }));
	ASSERT_EQ(value.size(), 2U);
	ASSERT_EQ(Trait::get(value).size(), 2U);
	ASSERT_EQ(Trait::get(std::as_const(value)).size(), 2U);
	ASSERT_EQ(Trait::add(value, 3), 3);
	ASSERT_EQ(value.size(), 3U);
	Trait::reserve(value, 100);
	ASSERT_GE(value.capacity(), 100U);
	ASSERT_EQ(Trait::size(std::as_const(value)), 3U);
	int sum = 0;
	Trait::apply(value, [&sum](int x) { sum += x; });
	ASSERT_EQ(sum, 6);
}

TEST(ObjectTrait, stackAllocatedVector) {
	using Vector = cpv::StackAllocatedVector<int, 3>;
	using Trait = cpv::ObjectTrait<Vector>;
	Vector value = Trait::create(Vector({ 1, 2, 3 }));
	ASSERT_EQ(value.size(), 3U);
	Trait::reset(value);
	ASSERT_EQ(value.size(), 0U);
	Trait::set(value, Vector({ 1, 2 }));
	ASSERT_EQ(value.size(), 2U);
	ASSERT_EQ(Trait::get(value).size(), 2U);
	ASSERT_EQ(Trait::get(std::as_const(value)).size(), 2U);
	ASSERT_EQ(Trait::add(value, 3), 3);
	ASSERT_EQ(value.size(), 3U);
	Trait::reserve(value, 100);
	ASSERT_GE(value.capacity(), 100U);
	ASSERT_EQ(Trait::size(std::as_const(value)), 3U);
	int sum = 0;
	Trait::apply(value, [&sum](int x) { sum += x; });
	ASSERT_EQ(sum, 6);
}

