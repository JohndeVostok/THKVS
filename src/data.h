#ifndef DATA_H
#define DATA_H

#include "msg_queue.hpp"
#include "crc.hpp"
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;


class Data {
public:
	Data(Data const&) = delete;
	void operator = (Data const&) = delete;
	static Data* getInstance()  {
		static Data _inst;
		return &_inst;
	}
	class KeyValue
	{
	public:
		int id, op;
		string ip;
		int port;
		string key, value;
		KeyValue();
		KeyValue(int _id, string _ip, int _port, string _key, string _value, int _op)
		{
			op = _op;
			ip = _ip;
			port = _port;
			id = _id;
			key = _key;
			value = _value;
		}
	};

	struct Value
	{
		int id;
		unsigned int key;
		string value;
		long long time_stamp;
		friend bool operator < (Value a, Value b)
		{
			return a.key < b.key;
		}
	};

	struct Hashlist
	{
		unsigned int begin, end;
		int cnt;
	};

	vector<Hashlist> hl;

	string conkey(unsigned int key);
	string conkey2(long long key);
	void get(int id, string ip, int port, string key);
	void put(int id, string ip, int port, string key, string value);
	void get_return(int id, string ip, int port, int status, string value, long long time_stamp);
	void put_return(int id, string ip, int port, int status);
	void run();

	long long tolonglong(string key);
	unsigned int toint(string key);

private:
	unordered_map<string, Value> umap;
	MsgQueue<KeyValue> que;

	Data() {};
	~Data() {};
};

#endif
