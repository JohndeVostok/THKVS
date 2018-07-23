#include "driver.h"
#include "crc.hpp"
#include <iostream>
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
}

Driver::~Driver() {}

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
	initSync(id);
	for (int hostIdx : hosts) {
		auto &host = hostList[hostIdx];
		thread t(&Driver::sendPut, this, ref(key), ref(value), ref(host), id);
		t.detach();
	}
	bool flag = true;
	while (flag) {
		mu.lock();
		if (suc >= THKVS_W || tot - suc > THKVS_N - THKVS_W) {
			flag = false;
		}
		mu.unlock();
	}
	return 0;
}

int Driver::get(string &key, string &value) {
	unsigned id = opid++;
	vector <int> hosts;
	getHosts(key, hosts);
	initSync(id);
	for (int hostIdx : hosts) {
		auto &host = hostList[hostIdx];
		thread t(&Driver::sendGet, this, ref(key), ref(value), ref(host), id);
		t.detach();
	}
	bool flag = true;
	while (flag) {
		mu.lock();
		if (suc >= THKVS_R || tot - suc > THKVS_N - THKVS_R) {
			flag = false;
		}
		mu.unlock();
	}
	return 0;
}

int Driver::initSync(int id) {
	mu.lock();
	cid = id;
	suc = 0;
	tot = 0;
	mu.unlock();
	return 0;
}

void Driver::sendPut(string &key, string &value, Host &host, int id) {
	//send()
	//recv()
	mu.lock();
	if (id == cid) {
		tot++;
		suc++;
	}
	mu.unlock();
}

void Driver::sendGet(string &key, string &value, Host &host, int id) {
	//send()
	//recv()
	mu.lock();
	if (id == cid) {
		tot++;
		suc++;
	}
	mu.unlock();
}

void Driver::test() {
	for (auto &host : hostList) {
		cout << host.host << host.count << endl;
	}
}
