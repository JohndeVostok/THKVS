#include "driver.h"
#include "crc.hpp"
#include <fstream>
#include <sstream>
#include <unordered_set>

Driver::Driver() {
	ifstream fin("config");
	Host tmp;
	while (fin >> tmp.host >> tmp.ip) {
		hostList.emplace_back(tmp);
	}
	for (int i = 0; i < hostList.size(); i++) {
		auto &host = hostList[i];
		ostringstream buf;
		string tmpstr;
		unsigned nodehash;
		for (int j = 0; j < (NODECOPY); j++) {
			buf.str("");
			buf << host.host << "#" << j;
			tmpstr = buf.str();
			nodehash = hash(tmpstr);
			nodeMap.emplace(nodehash, i);
		}
	}
	fin.close();
	hostCount.resize(hostList.size());
}

Driver::~Driver() {}

unsigned Driver::hash(string &str) {
	return getCRC(str);
}

void Driver::getServers(string &key, vector <int> &servers) {
	servers.clear();
	unsigned keyhash = hash(key);
	auto iter = nodeMap.lower_bound(keyhash);
	while (servers.size() < 3) {
		int flag = 0;
		for (auto &t : servers) {
			if (t == iter->second) {
				flag = 1;
			}
		}
		if (!flag) {
			servers.emplace_back(iter->second);
			hostCount[iter->second]++;
		}
		if (iter == nodeMap.end()) {
			iter = nodeMap.begin();
		} else {
			iter++;
		}
	}
}

int Driver::put(string &key, string &value) {
	vector <int> s;
	for (int i = 0; i < (1 << 16); i++) {
		ostringstream buf;
		buf << rand();
		key = buf.str();
		getServers(key, s);
	}
	for (int i = 0; i < hostCount.size(); i++) printf("%d\n", hostCount[i]);
}
