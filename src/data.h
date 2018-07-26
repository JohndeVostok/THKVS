#ifndef DATA_H
#define DATA_H

#include <string>
#include <map>
#include <cstdio>

using namespace std;


class Data {
public:
	Data();
	~Data();
	void get(int id, string key);
	void put(int id, string key, string value);
	void get_return(int id, string status, string value, long long time_stamp);
	void put_return(int id, string value);
	void run();

private:
	Data(Data const&) = delete;
	void operator = (Data const&) = delete;
	static Data* getInstance()  {
		static Data _inst;
		return &_inst;
	}
};

#endif
