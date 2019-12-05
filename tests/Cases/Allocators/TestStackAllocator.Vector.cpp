#include <functional>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	template <template <class, std::size_t> class VectorType>
	void testVector() {
		VectorType<std::string, 3> vec({ "a", "b", "c" });
		VectorType<std::string, 3> vecCopy(vec);
		VectorType<std::string, 3> vecMove(std::move(vec));
		VectorType<std::string, 3> vecCopyAssign;
		VectorType<std::string, 3> vecMoveAssignFrom;
		VectorType<std::string, 3> vecMoveAssign;
		VectorType<std::string, 3> vecAppend;
		
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

	template <template <class, std::size_t> class VectorType>
	void testVectorReset() {
		VectorType<int, 3> vec;
		void* begin = static_cast<void*>(&vec);
		void* end = static_cast<void*>(&vec + 1);
		for (std::size_t i = 0; i < 5; ++i) {
			vec.clear();
			for (std::size_t j = 0; j < 3; ++j) {
				vec.emplace_back(j);
				ASSERT_GE(static_cast<void*>(&vec.at(j)), begin);
				ASSERT_LT(static_cast<void*>(&vec.at(j)), end);
			}
		}
	}
}

TEST(StackAllocator, vector) {
	testVector<cpv::StackAllocatedVector>();
}

TEST(StackAllocator, vectorReset) {
	testVectorReset<cpv::StackAllocatedVector>();
}

