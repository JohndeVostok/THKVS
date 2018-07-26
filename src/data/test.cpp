#include "data.h"
#include <cstdio>
#include <iostream>

using namespace std;

int main()
{
	Data d;
	string key = "fuck";
	string value = "you";
	d.put(123, key, value);
	key = "your";
	value = "asshole";
	d.put(234, key, value);
	key = "fucking";
	d.get(1241252, key);
	key = "your";
	d.get(42847, key);
	d.run();
}
