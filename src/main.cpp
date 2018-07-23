#include "driver.h"
#include <sstream>

int main() {
	Driver driver;
	string key, value;
	key = "a";
	value = "aa";
	driver.put(key, value);
	driver.get(key, value);
	key = "b";
	value = "bb";
	driver.put(key, value);
	driver.get(key, value);
	return 0;
}
