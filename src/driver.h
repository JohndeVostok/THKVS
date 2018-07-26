#ifndef THKVS_DRIVER_H
#define THKVS_DRIVER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class Driver {
private:
	Driver();

	~Driver();
	struct Host {
		string host, ip;
		int port;
		int count = 0;
	};

	struct Node {
		string hostId;
	};

	struct SyncEntry {
		int id;
		int tot = 0, suc = 0;
		long long timestamp= -1;
		string value;
	};

	const int NODECOPY = 256;
	const int THKVS_N = 3; //replica number
	const int THKVS_R = 2; //read number
	const int THKVS_W = 2; //write number
	const int THKVS_TIMEOUT = 300;

	map <unsigned, int> nodeMap;
	unordered_map <int, SyncEntry> entries;
	vector <Host> hostList;
	Host localhost;

	//Sync
	unsigned opid = 0;
	mutex mu;

	unsigned hash(string &str);
	int getHosts(string &key, vector <int> &hosts);

public:
	Driver(Driver const&) = delete;
	void operator = (Driver const&) = delete;
	static Driver* getInstance()  {
		static Driver _inst;
		return &_inst;
	}
	int put(string &key, string &value);
	int putReturn(int id, int status);
	int putFinish(int id, int status);
	int get(string &key);
	int getReturn(int id, int status, long long timestamp, string &value);
	int getFinish(int id, int status, string &value);
	void test();
};

#endif
