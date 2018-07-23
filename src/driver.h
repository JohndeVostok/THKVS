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
	unsigned hash(string &str);
	void getServers(string &key, vector <int> &servers);
public:
	Driver();
	~Driver();
	void put();
	void get();
};

#endif