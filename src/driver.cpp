#include "driver.h"
#include "crc.hpp"
#include "msg_handler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

Driver::Driver() {
	ifstream fin("config");
	Host tmp;
	fin >> localhost.host >> localhost.ip >> localhost.port;
	while (fin >> tmp.host >> tmp.ip >> tmp.port) {
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
	bool flag;
	{
		lock_guard <mutex> lck(mu);
		flag = enableFlag;
	}
	if (flag) {
		putFinish(-1, 2);
	}
	SyncEntry entry;
	entry.tot = 0;
	entry.suc = 0;
	unsigned id;
	{
		lock_guard <mutex> lck(mu);
		id = opid++;
		entries.emplace(id, entry);
	}
	vector <int> hosts;
	getHosts(key, hosts);

	for (int hostIdx : hosts) {
		Host& host = hostList[hostIdx];
		std::cout << "[DEBUG] " << host.ip << " " <<  host.port << " " << key << " " << value << std::endl;
		msgHandler::sendPut(id, localhost.ip, localhost.port, host.ip, host.port, key, value);
	}
	std::cout << "[DEBUG] ended of put" << std::endl;
}

int Driver::putReturn(int id, int status) {
    std::cout << "[DEBUG DRIVER] Put Return id: " << id << " status: " << status << std::endl;

    int flag = 0;
	{
		lock_guard <mutex> lck(mu);
		if (entries.find(id) == entries.end()) {
			flag = 1;
		}
		if (!flag) {
			auto&& entry = entries[id];
			entry.tot++;
			entry.suc += status ^ 1;
    	    std::cout << "[DEBUG DRIVER] In putReturn: " << "tot: " << entry.tot << " suc: " << entry.suc << std::endl;
			if (entry.suc >= THKVS_W || entry.tot - entry.suc > THKVS_N - THKVS_W) {
				entries.erase(id);
				flag = 2 + status;
			}
		}
	}
	cond.notify_one();
	if (flag > 1) {
		putFinish(id, flag - 2);
	}
	return 0;
}

int Driver::putFinish(int id, int status) {
	cout << "[DEBUG DRIVER] PutFinish " << id << " " << status << endl;
}

int Driver::get(string &key) {
	bool flag;
	{
		lock_guard <mutex> lck(mu);
		flag = enableFlag;
	}
	if (flag) {
		string str = "";
		getFinish(-1, 2, str);
	}
	SyncEntry entry;
	entry.tot = 0;
	entry.suc = 0;
	unsigned id;
	{
		lock_guard <mutex> lck(mu);
		id = opid++;
		entries.emplace(id, entry);
	}
	std::cout << "[DEBUG DRIVER] Before Get id: " << id << std::endl;
	vector <int> hosts;
	getHosts(key, hosts);
	for (int hostIdx : hosts) {
		Host& host = hostList[hostIdx];
		msgHandler::sendGet(id, localhost.ip, localhost.port, host.ip, host.port, key);
	}
}

int Driver::getReturn(int id, int status, long long timestamp, string &value) {
    std::cout << "[DEBUG DRIVER] Get Return id: " << id << " status: " << status << "value: " << value << std::endl;
	int flag = 0;
	string str;
{
	std::lock_guard<std::mutex> lck(mu);
	if (entries.find(id) == entries.end()) {
		flag = 1;
	}
	std::cout << "[DEBUG DRIVER] In getReturn: flag: " << flag << std::endl;
	if (!flag) {
		//SyncEntry entry = entries[id];
		auto&& entry = entries[id];
		entry.tot++;
		entry.suc += status ^ 1;
		if (!status) {
			if (timestamp > entry.timestamp) {
				entry.timestamp = timestamp;
				entry.value = value;
			}
		}
        std::cout << "[DEBUG DRIVER] In getReturn: " << "tot: " << entry.tot << " suc: " << entry.suc << std::endl;
		if (entry.suc >= THKVS_R || entry.tot - entry.suc > THKVS_N - THKVS_R) {
			entries.erase(id);
			str = entry.value;
			flag = 2 + status;
		}
	}

}
	cond.notify_one();
    if (flag > 1) {
		getFinish(id, flag - 2, str); // entry.timestamp. entry.value;
	}
	return 0;
}

int Driver::getFinish(int id, int status, string &value) {
	cout << "[DEBUG DRIVER] GetFinish " << id << " " << status << " " << value << endl;
}

void Driver::test() {
	for (auto &host : hostList) {
		cout << host.host << host.count << endl;
	}
}

int Driver::setEnableFlag(bool flag) {
	unsigned id;
	{
		lock_guard <mutex> lck(mu);
		id = enableFlagEntry.id = opid++;
		enableFlagEntry.cnt = 0;
	}
	for (auto &host : hostList) {
		msgHandler::sendSetEnableFlag(id, localhost.ip, localhost.port, host.ip, host.port, flag);
	}
}

int Driver::actSetEnableFlag(bool flag) {
	unique_lock <mutex> lck(mu);
	cond.wait(lck, [this]() {return entries.empty();});
	enableFlag = flag;
}

int Driver::setEnableFlagReturn(int id, int status) {
	bool flag = 0;
	{
		lock_guard <mutex> lck(mu);
		enableFlagEntry.cnt++;
		if (enableFlagEntry.cnt == hostList.size()) flag = 1;
	}
	if (flag) {
		setEnableFlagFinish(id, 0);
	}
	return 0;
}

int Driver::setEnableFlagFinish(int id, int status) {
	cout << "[DEBUG SET ENABLE FLAG] id: " << id << " status: " << status << endl;
	return 0;
}
