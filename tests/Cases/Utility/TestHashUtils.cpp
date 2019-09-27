#include <string>
#include <unordered_map>
#include <CPVFramework/Utility/HashUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestHashUtils, hash) {
	{
		std::unordered_map<std::tuple<>, int, cpv::hash<std::tuple<>>> map;
		map.emplace(std::tuple<>(), 123);
		ASSERT_TRUE(map.find(std::tuple<>()) != map.end());
		ASSERT_EQ(map.at(std::tuple<>()), 123);
	}
	{
		std::unordered_map<std::tuple<int>, int, cpv::hash<std::tuple<int>>> map;
		map.emplace(std::tuple<int>(1), 101);
		map.emplace(std::tuple<int>(2), 102);
		ASSERT_TRUE(map.find(std::tuple<int>(1)) != map.end());
		ASSERT_TRUE(map.find(std::tuple<int>(2)) != map.end());
		ASSERT_TRUE(map.find(std::tuple<int>(3)) == map.end());
		ASSERT_EQ(map.at(std::tuple<int>(1)), 101);
		ASSERT_EQ(map.at(std::tuple<int>(2)), 102);
	}
	{
		std::unordered_map<std::tuple<int, std::string>, int,
			cpv::hash<std::tuple<int, std::string>>> map;
		map.emplace(std::tuple<int, std::string>(123, "a"), 101);
		map.emplace(std::tuple<int, std::string>(123, "b"), 102);
		ASSERT_TRUE(map.find(std::tuple<int, std::string>(123, "a")) != map.end());
		ASSERT_TRUE(map.find(std::tuple<int, std::string>(123, "b")) != map.end());
		ASSERT_TRUE(map.find(std::tuple<int, std::string>(123, "c")) == map.end());
		ASSERT_EQ(map.at(std::tuple<int, std::string>(123, "a")), 101);
		ASSERT_EQ(map.at(std::tuple<int, std::string>(123, "b")), 102);
	}
	{
		std::unordered_map<std::tuple<int, int, std::string>, int,
			cpv::hash<std::tuple<int, int, std::string>>> map;
		map.emplace(std::tuple<int, int, std::string>(123, -1, "a"), 101);
		map.emplace(std::tuple<int, int, std::string>(123, -1, "b"), 102);
		ASSERT_TRUE(map.find(std::tuple<int, int, std::string>(123, -1, "a")) != map.end());
		ASSERT_TRUE(map.find(std::tuple<int, int, std::string>(123, -1, "b")) != map.end());
		ASSERT_TRUE(map.find(std::tuple<int, int, std::string>(123, -1, "c")) == map.end());
		ASSERT_EQ(map.at(std::tuple<int, int, std::string>(123, -1, "a")), 101);
		ASSERT_EQ(map.at(std::tuple<int, int, std::string>(123, -1, "b")), 102);
	}
	{
		std::unordered_map<std::pair<int, std::string>, int,
			cpv::hash<std::pair<int, std::string>>> map;
		map.emplace(std::pair<int, std::string>(123, "a"), 101);
		map.emplace(std::pair<int, std::string>(123, "b"), 102);
		ASSERT_TRUE(map.find(std::pair<int, std::string>(123, "a")) != map.end());
		ASSERT_TRUE(map.find(std::pair<int, std::string>(123, "b")) != map.end());
		ASSERT_TRUE(map.find(std::pair<int, std::string>(123, "c")) == map.end());
		ASSERT_EQ(map.at(std::pair<int, std::string>(123, "a")), 101);
		ASSERT_EQ(map.at(std::pair<int, std::string>(123, "b")), 102);
	}
}

