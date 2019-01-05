#include <CPVFramework/Utility/Object.hpp>
#include <TestUtility/GTestUtils.hpp>
#include <seastar/core/shared_ptr.hh>

namespace {
	struct Foo {
		bool isFree = true;
		bool isReset = false;
		void freeResources() { isFree = true; }
		void reset() { isReset = true; }
	};

	struct Base { };

	struct Derived : public Base {
		seastar::shared_ptr<int> record_;
		void freeResources() { ++*record_; record_ = nullptr; }
		void reset(const seastar::shared_ptr<int>& record) { record_ = record; }
	};

	struct A { int a; };
	struct B { int b; };
	struct C : A, B {
		int c;
		static void freeResources() { }
		static void reset() { }
	};
}

TEST(TestObject, Simple) {
	for (std::size_t i = 0; i < 3; ++i) {
		auto foo = cpv::makeObject<Foo>();
		ASSERT_TRUE(foo.get());
		ASSERT_TRUE(foo->isFree);
		ASSERT_TRUE(foo->isReset);
		foo->isFree = false;
		foo->isReset = false;
	}
}

TEST(TestObject, UpCasting) {
	auto record = seastar::make_shared<int>(0);
	for (std::size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(*record, i);
		{
			auto base = cpv::makeObject<Derived>(record).cast<Base>();
		}
		ASSERT_EQ(*record, i+1);
	}
}

TEST(TestObject, DownCasting) {
	auto record = seastar::make_shared<int>(0);
	for (std::size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(*record, i);
		{
			auto base = cpv::makeObject<Derived>(record).cast<Base>();
			auto derived = std::move(base).cast<Derived>();
		}
		ASSERT_EQ(*record, i+1);
	}
}

TEST(TestObject, InvalidCasting) {
	cpv::makeObject<C>().cast<A>();
	ASSERT_THROWS_CONTAINS(
		cpv::LogicException,
		cpv::makeObject<C>().cast<B>(),
		"cast cause pointer address changed");
}

TEST(TestObject, moveAssignment) {
	auto record = seastar::make_shared<int>(0);
	for (std::size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(*record, i);
		{
			auto a = cpv::makeObject<Derived>(record);
			auto b = cpv::Object<Derived>(nullptr).cast<Base>();
			cpv::Object<Derived> c(nullptr);
			// cppcheck-suppress redundantAssignment
			b = std::move(a).cast<Base>();
			b = std::move(b);
			c = std::move(b).cast<Derived>();
			c = std::move(c);
			ASSERT_TRUE(a == nullptr);
			ASSERT_TRUE(b == nullptr);
			ASSERT_TRUE(c != nullptr);
		}
		ASSERT_EQ(*record, i+1);
	}
}

