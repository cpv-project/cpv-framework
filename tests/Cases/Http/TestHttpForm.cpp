#include <CPVFramework/Http/HttpForm.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(HttpForm, getaddclear) {
	cpv::HttpForm form;
	ASSERT_TRUE(form.getAll().empty());
	ASSERT_TRUE(form.get("a").empty());
	ASSERT_TRUE(form.getMany("a").empty());

	form.add("a", "1");
	form.add("b", "2");
	form.add("b", "3");
	ASSERT_EQ(form.getAll().size(), 2U);
	ASSERT_EQ(form.get("a"), "1");
	ASSERT_EQ(form.get("b"), "2");
	ASSERT_EQ(form.get("c"), "");
	ASSERT_EQ(form.getMany("a").size(), 1U);
	ASSERT_EQ(form.getMany("a").at(0), "1");
	ASSERT_EQ(form.getMany("b").size(), 2U);
	ASSERT_EQ(form.getMany("b").at(0), "2");
	ASSERT_EQ(form.getMany("b").at(1), "3");
	ASSERT_EQ(form.getMany("c").size(), 0U);

	form.clear();
	ASSERT_TRUE(form.getAll().empty());
	ASSERT_TRUE(form.get("a").empty());
	ASSERT_TRUE(form.getMany("a").empty());
}

TEST(HttpForm, parseUrlEncoded) {
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("key_a=value_1&key_b=value_2&key_b=value_3");
		ASSERT_EQ(form.getAll().size(), 2U);
		ASSERT_EQ(form.getMany("key_a").size(), 1U);
		ASSERT_EQ(form.getMany("key_a").at(0), "value_1");
		ASSERT_EQ(form.getMany("key_b").size(), 2U);
		ASSERT_EQ(form.getMany("key_b").at(0), "value_2");
		ASSERT_EQ(form.getMany("key_b").at(1), "value_3");
	}
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("key+a=value+1&key%20b=%E4%B8%80%E4%BA%8C%E4%B8%89");
		ASSERT_EQ(form.getAll().size(), 2U);
		ASSERT_EQ(form.getMany("key a").size(), 1U);
		ASSERT_EQ(form.getMany("key a").at(0), "value 1");
		ASSERT_EQ(form.getMany("key b").size(), 1U);
		ASSERT_EQ(form.getMany("key b").at(0), "一二三");
	}
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("");
		ASSERT_EQ(form.getAll().size(), 0U);
	}
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("a");
		ASSERT_EQ(form.getAll().size(), 1U);
		ASSERT_EQ(form.getMany("").size(), 1U);
		ASSERT_EQ(form.getMany("").at(0), "a");
	}
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("a=");
		ASSERT_EQ(form.getAll().size(), 1U);
		ASSERT_EQ(form.getMany("a").size(), 1U);
		ASSERT_EQ(form.getMany("a").at(0), "");
	}
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("&");
		ASSERT_EQ(form.getAll().size(), 1U);
		ASSERT_EQ(form.getMany("").size(), 1U);
		ASSERT_EQ(form.getMany("").at(0), "");
	}
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("a=&");
		ASSERT_EQ(form.getAll().size(), 1U);
		ASSERT_EQ(form.getMany("a").size(), 1U);
		ASSERT_EQ(form.getMany("a").at(0), "");
	}
	{
		cpv::HttpForm form;
		form.parseUrlEncoded("a=1&a=&a=3");
		ASSERT_EQ(form.getAll().size(), 1U);
		ASSERT_EQ(form.getMany("a").size(), 3U);
		ASSERT_EQ(form.getMany("a").at(0), "1");
		ASSERT_EQ(form.getMany("a").at(1), "");
		ASSERT_EQ(form.getMany("a").at(2), "3");
	}
}

TEST(HttpForm, buildUrlEncoded) {
	cpv::HttpForm form;
	form.add("key 1", "value 1");
	form.add("key_2", "value_2");
	form.add("key_2", "一二三");
	cpv::Packet p;
	form.buildUrlEncoded(p);
	ASSERT_EQ(p.toString(),
		"key%201=value%201&key_2=value_2&key_2=%E4%B8%80%E4%BA%8C%E4%B8%89");
}

