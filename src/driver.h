#ifndef THKVS_DRIVER_H
#define THKVS_DRIVER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>

using namespace std;

class Driver {
private:
	struct Host {
		string host, ip;
		int count = 0;
	};

	struct Node {
		string hostId;
	};

	const int NODECOPY = 256;
	const int HASHBASE = 31;
	const int THKVS_N = 3; //replica number
	const int THKVS_R = 2; //read number
	const int THKVS_W = 2; //write number
	const int THKVS_TIMEOUT = 300;

	map <unsigned, int> nodeMap;
	vector <Host> hostList;

	//sync
	unsigned opid = 0, cid, tot, suc;
	mutex mu;
	

	unsigned hash(string &str);
	int getHosts(string &key, vector <int> &hosts);
	int initSync(int id);
	void sendPut(string &key, string &value, Host &host, int id);
	void sendGet(string &key, string &value, Host &host, int id);
public:
	Driver();
	~Driver();
	int put(string &key, string &value);
	int get(string &key, string &value);
	void test();
};

#endif
