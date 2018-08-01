#include "data.h"
#include <cstdio>
#include <ctime>
#include <thread>

string g(int l)
{
	string r = "";
	while (l --)
	{
		r = r + char(rand() % 1 + 'a');
	}
	return r;
}

string to_str(int key)
{
	string ret = "";
	if (key == 0)
		return "0";
	while (key > 0)
	{
		ret = char(key % 10 + 48) + ret;
		key = key / 10;
	}
	return ret;
}

int main() {
	srand(time(0));
    auto thread_data = std::thread(&Data::run, Data::getInstance());

	int n = 100;
	while (n--)
	{
		puts("");
		printf("n:%d\n", n);
		puts("");
		/*
		string key = g(1);
		string value = g(100);
		*/
		string key = "a";
		key = key + to_str(n);
		string value = "b";
		value = value + to_str(n);

		printf("key:%s value:%s\n", key.c_str(), value.c_str());
		Data::getInstance()->put(123, "0.0", 0, key, value);
		/*
		Data::getInstance()->get(29579, "0.0", 0, key);
		key = g(20);
		Data::getInstance()->get(29579, "0.0", 0, key);
		key = g(200);
		Data::getInstance()->get(29579, "0.0", 0, key);
		key = g(200);
		Data::getInstance()->get(29579, "0.0", 0, key);
		*/
	}
	string key = "a";
	key = key + to_str(4);
	Data::getInstance()->get(29579, "0.0", 0, key);
	thread_data.join();
}
