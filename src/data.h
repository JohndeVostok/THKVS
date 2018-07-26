#ifndef DATA_H
#define DATA_H

#include "network/msg_queue.hpp"
#include <string>
#include <unordered_map>

using namespace std;


class Data {
public:
	class KeyValue
	{
	public:
		int id, op;
		string key, value;
		KeyValue();
		KeyValue(int _id, string _key, string _value, int _op)
		{
			op = _op;
			id = _id;
			key = _key;
			value = _value;
		}
	};

	struct Value
	{
		int id;
		string value;
		long long time_stamp;
	};

	Data();
	~Data();
	void get(int id, string key);
	void put(int id, string key, string value);
	void get_return(int id, string status, string value, long long time_stamp);
	void put_return(int id, string value);
	void run();

private:
	unordered_map<string, Value> umap;
	MsgQueue<KeyValue> que;

	Data(Data const&) = delete;
	void operator = (Data const&) = delete;
	static Data* getInstance()  {
		static Data _inst;
		return &_inst;
	}
};

#endif
