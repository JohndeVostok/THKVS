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
		ostringstream oss;
		string tmpstr;
		unsigned nodehash;
		for (int j = 0; j < (NODECOPY); j++) {
			oss.str("");
			oss << host.host << "#" << j;
			tmpstr = oss.str();
			nodehash = hash(tmpstr);
			nodeMap.emplace(nodehash, i);
		}
	}
	fin.close();
}

Driver::~Driver() {}

unsigned Driver::hash(string &str) {
	return getCRC(str);
}

void Driver::getServers(string &key, vector <int> &servers) {
	servers.clear();
	unsigned keyhash = hash(key);
	printf("%u\n", keyhash);
	auto iter = nodeMap.lower_bound(keyhash);
	while (servers.size() < 3) {
		printf("%u %d\n", iter->first, iter->second);
		int flag = 0;
		for (auto &t : servers) {
			if (t == iter->second) {
				flag = 1;
			}
		}
		if (!flag) {
			servers.emplace_back(iter->second);
		}
		if (iter == nodeMap.end()) {
			iter = nodeMap.begin();
		} else {
			iter++;
		}
	}
}

void Driver::put() {
	vector <int> s;
	string str = "sb";
	getServers(str, s);
	for (auto &t : s) {
		printf("%d ", t);
	}
	printf("\n");
	str = "sc";
	getServers(str, s);
	for (auto &t : s) {
		printf("%d ", t);
	}
	printf("\n");
	str = "sd";
	getServers(str, s);
	for (auto &t : s) {
		printf("%d ", t);
	}
	printf("\n");
}
