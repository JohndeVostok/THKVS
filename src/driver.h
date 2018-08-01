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
#include <condition_variable>
#include <atomic>

using namespace std;

class Driver {
private:
	struct Host {
		string hostname, ip;
		int port;
		int count = 0;
	};

	struct Node {
		string hostId;
	};

	struct SyncEntry {
		int tot = 0, suc = 0;
		long long timestamp = -1;
		string value;
	};

	struct EnableFlagEntry {
		int cnt;
		int id = -1;
	};

	const int NODECOPY = 2;
	const int THKVS_N = 3; //replica number
	const int THKVS_R = 2; //read number
	const int THKVS_W = 2; //write number
	const int THKVS_TIMEOUT = 300;

	map <unsigned, int> nodeMap;
	unordered_map <int, SyncEntry> entries;
	vector <Host> hostList;
	Host localhost;
	EnableFlagEntry enableFlagEntry;

	//Sync
	bool enableFlag = 0;
	condition_variable condEntries, condServer, condEnable, condMove;
	mutex mu;
	atomic <unsigned> opid{0};
	atomic <int> serverCnt{0};
	atomic <int> enableCnt{0};
	atomic <int> moveCnt{0};

	Driver();
	~Driver();
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
	int setEnableFlag(bool flag);
	int actSetEnableFlag(bool flag);
	int setEnableFlagReturn(int id, int status);
	int setEnableFlagFinish(int id, int status);
	int addServer(string &hostname, string &ip, int port);
	int actAddServer(string &hostname, string &ip, int port);
	int addServerReturn(int id, int status);
	int removeServer(string &hostname);
	int actRemoveServer(string &hostname);
	int removeServerReturn(int id, int status);
	int moveReturn(int id, int status);
	int test();
};

#endif
