#include <CPVFramework/Utility/UUIDUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestUUIDUtils, strToUUID) {
	cpv::UUIDDataType uuid = cpv::strToUUID("00112233-4455-6677-8899-aabbccddeeff");
	ASSERT_EQ(uuid.first, 0x0011223344556677U);
	ASSERT_EQ(uuid.second, 0x8899aabbccddeeffU);
}

TEST(TestUUIDUtils, uuidToStr) {
	cpv::UUIDDataType uuid(0x12345678abcdefaaU, 0x87654321ffabcdefU);
	ASSERT_EQ(cpv::uuidToStr(uuid), "12345678-ABCD-EFAA-8765-4321FFABCDEF");
}

TEST(TestUUIDUtils, makeEmptyUUID) {
	cpv::UUIDDataType emptyUUID = cpv::makeEmptyUUID();
	ASSERT_EQ(cpv::uuidToStr(emptyUUID), "00000000-0000-0000-0000-000000000000");
	ASSERT_EQ(emptyUUID, cpv::makeEmptyUUID());
}

TEST(TestUUIDUtils, makeRandomUUID) {
	cpv::UUIDDataType a = cpv::makeRandomUUID();
	cpv::UUIDDataType b = cpv::makeRandomUUID();
	ASSERT_FALSE(a == b);
	ASSERT_EQ(a.first & 0xf000, 0x4000U); // version
	ASSERT_EQ(a.second & 0xc000'0000'0000'0000, 0x8000'0000'0000'0000U); // variant
}

TEST(TestUUIDUtils, makeTimeUUID) {
	cpv::UUIDDataType a = cpv::makeTimeUUID();
	cpv::UUIDDataType b = cpv::makeTimeUUID();
	ASSERT_EQ(cpv::getVersionFromUUID(a), cpv::TimeUUIDVersion);
	ASSERT_TRUE(a < b);
}

TEST(TestUUIDUtils, makeMinTimeUUID) {
	cpv::UUIDDataType uuid = cpv::makeTimeUUID();
	cpv::UUIDDataType minUUID = cpv::makeMinTimeUUID(cpv::getTimeFromUUID(uuid));
	ASSERT_GE(uuid.first, minUUID.first);
	ASSERT_GE(uuid.second, minUUID.second);
	ASSERT_EQ(minUUID.second, 0x8000'0000'0000'0000ULL);
}

TEST(TestUUIDUtils, makeMaxTimeUUID) {
	cpv::UUIDDataType uuid = cpv::makeTimeUUID();
	cpv::UUIDDataType maxUUID = cpv::makeMaxTimeUUID(cpv::getTimeFromUUID(uuid));
	ASSERT_LE(uuid.first, maxUUID.first);
	ASSERT_LE(uuid.second, maxUUID.second);
	ASSERT_EQ(maxUUID.second, 0xbfff'ffff'ffff'ffffULL);
}

TEST(TestUUIDUtils, getVersionFromUUID) {
	ASSERT_EQ(cpv::getVersionFromUUID(cpv::makeEmptyUUID()), cpv::EmptyUUIDVersion);
	ASSERT_EQ(cpv::getVersionFromUUID(cpv::makeRandomUUID()), cpv::RandomUUIDVersion);
	ASSERT_EQ(cpv::getVersionFromUUID(cpv::makeTimeUUID()), cpv::TimeUUIDVersion);
	ASSERT_EQ(cpv::getVersionFromUUID(cpv::strToUUID("00112233-4455-6677-8899-aabbccddeeff")), 6);
}

TEST(TestUUIDUtils, getTimeFromUUID) {
	cpv::UUIDDataType uuid = cpv::makeTimeUUID();
	auto time = cpv::getTimeFromUUID(uuid);
	cpv::UUIDDataType minUUID = cpv::makeMinTimeUUID(time);
	ASSERT_EQ(cpv::getTimeFromUUID(minUUID), time);
}

