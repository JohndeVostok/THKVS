#ifndef THKVS_DRIVER_H
#define THKVS_DRIVER_H

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>

using namespace std;

class Driver {
private:
	struct Host {
		string host, ip;
	};

	struct Node {
		string hostId;
	};

	const int NODECOPY = 256;
	const int HASHBASE = 31;

	map <unsigned, int> nodeMap;
	vector <Host> hostList;
	vector <int> hostCount;
	unsigned hash(string &str);
	void getServers(string &key, vector <int> &servers);
public:
	Driver();
	~Driver();
	int put(string &key, string &value);
	int get(string &key, string &value);
};

#endif
