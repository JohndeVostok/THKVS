#include "driver.h"
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
	unsigned x = 0, t = 1;
	int l = str.size();
	for (int i = 0; i < l; i++) {
		x += str[i] * t;
		t *= 31;
	}
	return x;
}

void Driver::getServers(string &key, vector <int> &servers) {
	servers.clear();
	string str = key + "salted";
	unsigned keyhash = hash(str);
	printf("%u\n", keyhash);
	auto iter = nodeMap.lower_bound(keyhash);
	for (int i = 0; i < 3; i++) {
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
	str = "sb";
	getServers(str, s);
	for (auto &t : s) {
		printf("%d ", t);
	}
	printf("\n");
}
