#include <functional>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	template <template <class, class, std::size_t> class MapType>
	void testMap() {
		MapType<int, std::string, 3> map({
			{ 100, "a" },
			{ 101, "b" },
			{ 102, "c" }
		});
		MapType<int, std::string, 3> mapCopy(map);
		MapType<int, std::string, 3> mapMove(std::move(map));
		MapType<int, std::string, 3> mapAppend;
		MapType<int, std::string, 3> mapCopyAssign;
		MapType<int, std::string, 3> mapMoveAssignFrom;
		MapType<int, std::string, 3> mapMoveAssign;
		
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
	
	template <template <class, class, std::size_t> class MapType>
	void testMapReset() {
		MapType<int, int, 3> map;
		void* begin = static_cast<void*>(&map);
		void* end = static_cast<void*>(&map + 1);
		for (std::size_t i = 0; i < 5; ++i) {
			map.clear();
			for (std::size_t j = 0; j < 3; ++j) {
				map.emplace(j, j);
				ASSERT_GE(static_cast<void*>(&map.at(j)), begin);
				ASSERT_LT(static_cast<void*>(&map.at(j)), end);
			}
		}
	}
	
	// test different initial sizes to verify alignment with sanitizers
	template <template <class, class, std::size_t> class MapType, std::size_t InitialSize>
	void testMapWithInitialSize() {
		MapType<int, int, InitialSize> map;
		for (std::size_t i = 0; i < InitialSize; ++i) {
			map.emplace(i, 100 + i);
		}
		for (std::size_t i = 0; i < InitialSize; ++i) {
			map.at(i) += i;
		}
	}
}

TEST(TestStackAllocator, unorderedMap) {
	testMap<cpv::StackAllocatedUnorderedMap>();
}

TEST(TestStackAllocator, unorderedMapReset) {
	testMapReset<cpv::StackAllocatedUnorderedMap>();
}

TEST(TestStackAllocator, unorderedMapWithInitialSize) {
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 1>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 2>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 3>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 4>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 5>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 6>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 7>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 8>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 9>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 10>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 11>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 12>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 13>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 14>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 15>();
	testMapWithInitialSize<cpv::StackAllocatedUnorderedMap, 16>();
}

TEST(TestStackAllocator, map) {
	testMap<cpv::StackAllocatedMap>();
}

TEST(TestStackAllocator, mapReset) {
	testMapReset<cpv::StackAllocatedMap>();
}

TEST(TestStackAllocator, mapWithInitialSize) {
	testMapWithInitialSize<cpv::StackAllocatedMap, 1>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 2>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 3>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 4>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 5>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 6>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 7>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 8>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 9>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 10>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 11>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 12>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 13>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 14>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 15>();
	testMapWithInitialSize<cpv::StackAllocatedMap, 16>();
}

