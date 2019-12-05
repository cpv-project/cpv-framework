#include <functional>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(StackAllocator, allocate) {
	static constexpr const std::size_t Size = sizeof(int)*4;
	cpv::StackAllocatorStorage<Size> storage;
	cpv::StackAllocator<int, Size> allocator(storage);
	
	using ptr_type = std::unique_ptr<int, std::function<void(int*)>>;
	ptr_type first(allocator.allocate(1), [&allocator] (int* p) { allocator.deallocate(p, 1); });
	ptr_type second(allocator.allocate(2), [&allocator] (int* p) { allocator.deallocate(p, 2); });
	ptr_type forth(allocator.allocate(1), [&allocator] (int* p) { allocator.deallocate(p, 1); });
	ptr_type fifth(allocator.allocate(1), [&allocator] (int* p) { allocator.deallocate(p, 1); });
	ptr_type sixth(allocator.allocate(2), [&allocator] (int* p) { allocator.deallocate(p, 2); });
	
	void* begin = static_cast<void*>(&storage);
	void* end = static_cast<void*>(&storage + 1);
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

