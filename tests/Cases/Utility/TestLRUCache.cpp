#include <string>
#include <CPVFramework/Utility/LRUCache.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestLRUCache, all) {
	cpv::LRUCache<int, std::string> cache(3);
	{
		cache.set(1, "a");
		cache.set(2, "b");
		cache.set(3, "c");
	}
	{
		std::string* result = cache.get(1);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "a");
		result = cache.get(2);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "b");
		result = cache.get(3);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "c");
		result = cache.get(-1);
		ASSERT_FALSE(result != nullptr);
		// list: 3, 2, 1
	}
	{
		cache.set(100, "abc");
		std::string* result = cache.get(1);
		ASSERT_FALSE(result != nullptr);
		result = cache.get(100);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "abc");
		result = cache.get(2);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "b");
		result = cache.get(3);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "c");
		// list: 3, 2, 100
	}
	{
		cache.set(101, "abc");
		std::string* result = cache.get(100);
		ASSERT_FALSE(result != nullptr);
		result = cache.get(101);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "abc");
		// list: 101, 3, 2
	}
	{
		cache.set(101, "def");
		std::string* result = cache.get(101);
		ASSERT_TRUE(result != nullptr);
		ASSERT_EQ(*result, "def");
		// list: 101, 3, 2
	}
	{
		ASSERT_EQ(cache.size(), 3U);
		ASSERT_FALSE(cache.empty());
		ASSERT_TRUE(cache.erase(2));
		ASSERT_TRUE(cache.erase(3));
		ASSERT_TRUE(cache.erase(101));
		ASSERT_FALSE(cache.erase(101));
		ASSERT_EQ(cache.size(), 0U);
		ASSERT_TRUE(cache.empty());
	}
}

