#include "driver.h"
#include <sstream>

int main() {
	string key, value;
	key = "a";
	value = "aa";
	Driver::getInstance()->put(key, value);
	Driver::getInstance()->get(key);
	return 0;
}
