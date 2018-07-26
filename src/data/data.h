#ifndef DATA_H
#define DATA_H

#include <string>
#include <map>
#include <cstdio>

using namespace std;

class Data {
public:
	void get(int id, string key);
	void put(int id, string key, string value);
	string get_return(int id, string status, string value, long long time_stamp);
	string put_return(int id, string value);
	void run();
};

#endif
