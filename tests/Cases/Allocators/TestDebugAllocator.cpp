#include <functional>
#include <CPVFramework/Allocators/DebugAllocator.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	template <class T>
	using DebugVector = std::vector<T, cpv::DebugAllocator<T, cpv::DebugAllocatorMode::NoLogging>>;
	template <class Key, class T>
	using DebugUnorderedMap = std::unordered_map<Key, T, std::hash<Key>, std::equal_to<Key>,
		cpv::DebugAllocator<std::pair<const Key, T>, cpv::DebugAllocatorMode::NoLogging>>;
}

TEST(TestDebugAllocator, vector) {
	DebugVector<std::string> vec({ "a", "b", "c" });
	DebugVector<std::string> vecCopy(vec);
	DebugVector<std::string> vecMove(std::move(vec));
	DebugVector<std::string> vecCopyAssign;
	DebugVector<std::string> vecMoveAssignFrom;
	DebugVector<std::string> vecMoveAssign;
	DebugVector<std::string> vecAppend;
	
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

TEST(TestDebugAllocator, unorderedMap) {
	DebugUnorderedMap<int, std::string> map({
		{ 100, "a" },
		{ 101, "b" },
		{ 102, "c" }
	});
	DebugUnorderedMap<int, std::string> mapCopy(map);
	DebugUnorderedMap<int, std::string> mapMove(std::move(map));
	DebugUnorderedMap<int, std::string> mapAppend;
	DebugUnorderedMap<int, std::string> mapCopyAssign;
	DebugUnorderedMap<int, std::string> mapMoveAssignFrom;
	DebugUnorderedMap<int, std::string> mapMoveAssign;
	
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

