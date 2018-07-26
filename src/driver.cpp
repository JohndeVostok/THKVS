#include "driver.h"
#include "crc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>

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
}

Driver::~Driver() {}

Driver::getInstance() {
	static Driver _inst;
	return &_inst;
}

unsigned Driver::hash(string &str) {
	return getCRC(str);
}

int Driver::getHosts(string &key, vector <int> &hosts) {
	hosts.clear();
	if (hostList.size() < THKVS_N) {
		return 1;
	}
	unsigned keyhash = hash(key);
	auto iter = nodeMap.lower_bound(keyhash);
	while (hosts.size() < 3) {
		int flag = 0;
		for (auto &t : hosts) {
			if (t == iter->second) {
				flag = 1;
			}
		}
		if (!flag) {
			hosts.emplace_back(iter->second);
			hostList[iter->second].count++;
		}
		if (iter == nodeMap.end()) {
			iter = nodeMap.begin();
		} else {
			iter++;
		}
	}
	return 0;
}

int Driver::put(string &key, string &value) {
	unsigned id = opid++;
	vector <int> hosts;
	getHosts(key, hosts);
	for (int hostIdx : hosts) {
		SyncEntry entry;
		mu.lock();
		entry.id = opid++;
		entries.emplace(id, entry);
		mu.unlock();
		//TODO : put
	}
}

int Driver::putReturn(int id, int status) {
	int flag = 0;
	mu.lock();
	if (entries.find(id) != entries.end()) {
		flag = 1;
	}
	if (!flag) {
		SyncEntry entry = entries[id];
		entry.tot++;
		entry.suc += status;
		if (entry.suc >= THKVS_W || entry.tot - entry.suc > THKVS_N - THKVS_W) {
			entries.erase(id);
			flag = 2 + status;
		}
	}
	mu.unlock();
	if (flag > 1) {
		putFinish(id);
	}
	return 0;
}

int Driver::putFinish(int id) {
	//TODO
}

int Driver::get(string &key, string &value) {
	unsigned id = opid++;
	vector <int> hosts;
	getHosts(key, hosts);
	for (int hostIdx : hosts) {
		SyncEntry entry;
		mu.lock();
		entry.id = opid++;
		entries.emplace(id, entry);
		mu.unlock();
		//TODO : get
	}
}

int Driver::getReturn(int id, int status, int timestamp, string &value) {
	int flag = 0;
	mu.lock();
	if (entries.find(id) != entries.end()) {
		flag = 1;
	}
	if (!flag) {
		SyncEntry entry = entries[id];
		entry.tot++;
		entry.suc += status;
		if (!status) {
			if (timestamp > entry.timestamp) {
				entry.timestamp = timestamp;
				entry.value = value;
			}
		}
		if (entry.suc >= THKVS_R || entry.tot - entry.suc > THKVS_N - THKVS_R) {
			entries.erase(id);
			flag = 2 + status;
		}
	}
	mu.unlock();
	if (flag > 1) {
		getFinish(id); // entry.timestamp. entry.valye;
	}
	return 0;
}

int Driver::getFinish(int id) {
	//TODO
}

void Driver::test() {
	for (auto &host : hostList) {
		cout << host.host << host.count << endl;
	}
}
