#include <functional>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestStackAllocator, allocate) {
	cpv::StackAllocator<int, 4> allocator;
	
	using ptr_type = std::unique_ptr<int, std::function<void(int*)>>;
	ptr_type first(allocator.allocate(1), [&allocator] (int* p) { allocator.deallocate(p, 1); });
	ptr_type second(allocator.allocate(2), [&allocator] (int* p) { allocator.deallocate(p, 2); });
	ptr_type forth(allocator.allocate(1), [&allocator] (int* p) { allocator.deallocate(p, 1); });
	ptr_type fifth(allocator.allocate(1), [&allocator] (int* p) { allocator.deallocate(p, 1); });
	ptr_type sixth(allocator.allocate(2), [&allocator] (int* p) { allocator.deallocate(p, 2); });
	
	void* begin = static_cast<void*>(&allocator);
	void* end = static_cast<void*>(&allocator + 1);
	ASSERT_GE(static_cast<void*>(first.get()), begin);
	ASSERT_LT(static_cast<void*>(first.get()), end);
	ASSERT_GE(static_cast<void*>(second.get()), begin);
	ASSERT_LT(static_cast<void*>(second.get()), end);
	ASSERT_GE(static_cast<void*>(forth.get()), begin);
	ASSERT_LT(static_cast<void*>(forth.get()), end);
	ASSERT_EQ(static_cast<void*>(first.get() + 1), static_cast<void*>(second.get()));
	ASSERT_EQ(static_cast<void*>(second.get() + 2), static_cast<void*>(forth.get()));
	ASSERT_TRUE(static_cast<void*>(fifth.get()) < begin || static_cast<void*>(fifth.get()) > end);
	ASSERT_TRUE(static_cast<void*>(sixth.get()) < begin || static_cast<void*>(sixth.get()) > end);
}

TEST(TestStackAllocator, vector) {
	cpv::StackAllocatedVector<std::string, 3> vec({ "a", "b", "c" });
	cpv::StackAllocatedVector<std::string, 3> vecCopy(vec);
	cpv::StackAllocatedVector<std::string, 3> vecMove(std::move(vec));
	cpv::StackAllocatedVector<std::string, 3> vecCopyAssign;
	cpv::StackAllocatedVector<std::string, 3> vecMoveAssignFrom;
	cpv::StackAllocatedVector<std::string, 3> vecMoveAssign;
	cpv::StackAllocatedVector<std::string, 3> vecAppend;
	
	vecCopy.at(2) = "c_";
	vecCopyAssign = vecCopy;
	vecMoveAssignFrom = vecCopy;
	vecMoveAssign = std::move(std::move(vecMoveAssignFrom));
	vecAppend.emplace_back("a");
	vecAppend.emplace_back("b");
	vecAppend.emplace_back("c");
	vecAppend.emplace_back("d");
	vecAppend.emplace_back("e");
	
	ASSERT_TRUE(vec.empty());
	ASSERT_EQ(vecCopy.at(0), "a");
	ASSERT_EQ(vecCopy.at(1), "b");
	ASSERT_EQ(vecCopy.at(2), "c_");
	ASSERT_EQ(vecMove.at(0), "a");
	ASSERT_EQ(vecMove.at(1), "b");
	ASSERT_EQ(vecMove.at(2), "c");
	ASSERT_EQ(vecCopyAssign.at(0), "a");
	ASSERT_EQ(vecCopyAssign.at(1), "b");
	ASSERT_EQ(vecCopyAssign.at(2), "c_");
	ASSERT_TRUE(vecMoveAssignFrom.empty());
	ASSERT_EQ(vecMoveAssign.at(0), "a");
	ASSERT_EQ(vecMoveAssign.at(1), "b");
	ASSERT_EQ(vecMoveAssign.at(2), "c_");
	ASSERT_EQ(vecAppend.at(0), "a");
	ASSERT_EQ(vecAppend.at(1), "b");
	ASSERT_EQ(vecAppend.at(2), "c");
	ASSERT_EQ(vecAppend.at(3), "d");
	ASSERT_EQ(vecAppend.at(4), "e");
}

TEST(TestStackAllocator, vectorReset) {
	cpv::StackAllocatedVector<int, 3> vec;
	void* begin = static_cast<void*>(&vec);
	void* end = static_cast<void*>(&vec + 1);
	for (std::size_t i = 0; i < 5; ++i) {
		vec = {};
		for (std::size_t j = 0; j < 3; ++j) {
			vec.emplace_back(j);
			ASSERT_GE(static_cast<void*>(&vec.at(j)), begin);
			ASSERT_LT(static_cast<void*>(&vec.at(j)), end);
		}
	}
}

TEST(TestStackAllocator, unorderedMap) {
	cpv::StackAllocatedUnorderedMap<int, std::string, 3> map({
		{ 100, "a" },
		{ 101, "b" },
		{ 102, "c" }
	});
	cpv::StackAllocatedUnorderedMap<int, std::string, 3> mapCopy(map);
	cpv::StackAllocatedUnorderedMap<int, std::string, 3> mapMove(std::move(map));
	cpv::StackAllocatedUnorderedMap<int, std::string, 3> mapAppend;
	cpv::StackAllocatedUnorderedMap<int, std::string, 3> mapCopyAssign;
	cpv::StackAllocatedUnorderedMap<int, std::string, 3> mapMoveAssignFrom;
	cpv::StackAllocatedUnorderedMap<int, std::string, 3> mapMoveAssign;
	
	mapCopy.at(102) = "c_";
	mapCopyAssign = mapCopy;
	mapMoveAssignFrom = mapCopy;
	mapMoveAssign = std::move(std::move(mapMoveAssignFrom));
	mapAppend.emplace(100, "a");
	mapAppend.emplace(101, "b");
	mapAppend.emplace(102, "c");
	mapAppend.emplace(103, "d");
	mapAppend.emplace(104, "e");
	
	ASSERT_TRUE(map.empty());
	ASSERT_EQ(mapCopy.at(100), "a");
	ASSERT_EQ(mapCopy.at(101), "b");
	ASSERT_EQ(mapCopy.at(102), "c_");
	ASSERT_EQ(mapMove.at(100), "a");
	ASSERT_EQ(mapMove.at(101), "b");
	ASSERT_EQ(mapMove.at(102), "c");
	ASSERT_EQ(mapCopyAssign.at(100), "a");
	ASSERT_EQ(mapCopyAssign.at(101), "b");
	ASSERT_EQ(mapCopyAssign.at(102), "c_");
	ASSERT_TRUE(mapMoveAssignFrom.empty());
	ASSERT_EQ(mapMoveAssign.at(100), "a");
	ASSERT_EQ(mapMoveAssign.at(101), "b");
	ASSERT_EQ(mapMoveAssign.at(102), "c_");
	ASSERT_EQ(mapAppend.at(100), "a");
	ASSERT_EQ(mapAppend.at(101), "b");
	ASSERT_EQ(mapAppend.at(102), "c");
	ASSERT_EQ(mapAppend.at(103), "d");
	ASSERT_EQ(mapAppend.at(104), "e");
}

TEST(TestStackAllocator, unorderedMapReset) {
	cpv::StackAllocatedUnorderedMap<int, int, 3> map;
	void* begin = static_cast<void*>(&map);
	void* end = static_cast<void*>(&map + 1);
	for (std::size_t i = 0; i < 5; ++i) {
		map = {};
		for (std::size_t j = 0; j < 3; ++j) {
			map.emplace(j, j);
			ASSERT_GE(static_cast<void*>(&map.at(j)), begin);
			ASSERT_LT(static_cast<void*>(&map.at(j)), end);
		}
	}
}

namespace {
	// test different initial sizes to verify alignment with sanitizers
	template <std::size_t InitialSize>
	void testStackAllocatedUnorderedMapWithInitialSize() {
		cpv::StackAllocatedUnorderedMap<int, int, InitialSize> map;
		for (std::size_t i; i < InitialSize; ++i) {
			map.emplace(i, 100 + i);
		}
		for (std::size_t i; i < InitialSize; ++i) {
			map.at(i) += i;
		}
	}
}

TEST(TestStackAllocator, unorderedMapWithInitialSize) {
	testStackAllocatedUnorderedMapWithInitialSize<1>();
	testStackAllocatedUnorderedMapWithInitialSize<2>();
	testStackAllocatedUnorderedMapWithInitialSize<3>();
	testStackAllocatedUnorderedMapWithInitialSize<4>();
	testStackAllocatedUnorderedMapWithInitialSize<5>();
	testStackAllocatedUnorderedMapWithInitialSize<6>();
	testStackAllocatedUnorderedMapWithInitialSize<7>();
	testStackAllocatedUnorderedMapWithInitialSize<8>();
	testStackAllocatedUnorderedMapWithInitialSize<9>();
	testStackAllocatedUnorderedMapWithInitialSize<10>();
	testStackAllocatedUnorderedMapWithInitialSize<11>();
	testStackAllocatedUnorderedMapWithInitialSize<12>();
	testStackAllocatedUnorderedMapWithInitialSize<13>();
	testStackAllocatedUnorderedMapWithInitialSize<14>();
	testStackAllocatedUnorderedMapWithInitialSize<15>();
	testStackAllocatedUnorderedMapWithInitialSize<16>();
}

