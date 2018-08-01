#include "driver.h"
#include "crc.h"
#include "msg_handler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

Driver::Driver() {
	ifstream fin("config");
	Host tmp;
	fin >> localhost.hostname >> localhost.ip >> localhost.port;
	while (fin >> tmp.hostname >> tmp.ip >> tmp.port) {
		hostList.emplace_back(tmp);
	}
	for (int i = 0; i < hostList.size(); i++) {
		auto &host = hostList[i];
		ostringstream buf;
		string tmpstr;
		unsigned nodehash;
		for (int j = 0; j < (NODECOPY); j++) {
			buf.str("");
			buf << host.hostname << "#" << j;
			tmpstr = buf.str();
			nodehash = hash(tmpstr);
			if (!nodeMap.count(nodehash)) {
				nodeMap.emplace(nodehash, i);
			}
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
	auto iter = nodeMap.upper_bound(keyhash);
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
		return 0;
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
	condEntries.notify_all();
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
		return 0;
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
		lock_guard <mutex> lck(mu);
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

	condEntries.notify_all();
    if (flag > 1) {
		getFinish(id, flag - 2, str); // entry.timestamp. entry.value;
	}
	return 0;
}

int Driver::getFinish(int id, int status, string &value) {
	cout << "[DEBUG DRIVER] GetFinish " << id << " " << status << " " << value << endl;
}

int Driver::setEnableFlag(bool flag) {
	unsigned id;
	{
		lock_guard <mutex> lck(mu);
		id = enableFlagEntry.id = opid++;
		enableFlagEntry.cnt = 0;
	}
	enableCnt.store(hostList.size());
	std::cout << "[DEBUG DRIVER] in setEnableFlag: enableCnt: " << enableCnt.load() << std::endl;
    for (auto &host : hostList) {
		msgHandler::sendSetEnableFlag(id, localhost.ip, localhost.port, host.ip, host.port, flag);
	}
	unique_lock <mutex> lck(mu);
	condEnable.wait(lck, [this]() {return enableCnt.load() == 0;});
}

int Driver::actSetEnableFlag(bool flag) {
	unique_lock <mutex> lck(mu);
	condEntries.wait(lck, [this]() {return entries.empty();});
	enableFlag = flag;
	std::cout << "[DEBUG DRIVER] in actSetEnableFlag: enableFlag: " << enableFlag << std::endl;
}

int Driver::setEnableFlagReturn(int id, int status) {
	enableCnt--;
	std::cout << "[DEBUG DRIVER] in setEnableFlagReturn: enableCnt: " << enableCnt.load() << std::endl;

	condEnable.notify_all();
	return 0;
}

int Driver::addServer(string &hostname, string &ip, int port) {
	bool flag;
	{
		lock_guard <mutex> lck(mu);
		flag = enableFlag;
	}
	if (flag) {
		cout << "[DEBUG DRIVER] in addServer trap disabled." << endl;
		return 1;
	}
	setEnableFlag(1);
	vector <Host> tmp;
	{
		lock_guard <mutex> lck(mu);
		tmp = hostList;
		serverCnt.store(tmp.size());
		unsigned id = opid++;
		for (auto &host : tmp) {
			msgHandler::sendAddServer(id, localhost.ip, localhost.port, host.ip, host.port, hostname, ip, port);
		}
	}
	std::cout << "[DEBUG DRIVER] before condServer.wait" << std::endl;
    {
        unique_lock<mutex> lck(mu);
        condServer.wait(lck, [this]() { return serverCnt.load() == 0; });
        std::cout << "[DEBUG DRIVER] after condServer.wait" << std::endl;
    }
	setEnableFlag(0);
}

int Driver::actAddServer(string &hostname, string &ip, int port) {
	lock_guard <mutex> lck(mu);
	Host tmp;
	tmp.hostname = hostname;
	tmp.ip = ip;
	tmp.port = port;
	hostList.emplace_back(tmp);

	auto &host = hostList.back();
	ostringstream buf;
	string tmpstr;
	unsigned nodehash;
	for (int i = 0; i < NODECOPY; i++) {
		buf.str("");
		buf << host.hostname << "#" << i;
		tmpstr = buf.str();
		nodehash = hash(tmpstr);
		if (!nodeMap.count(nodehash)) {
			nodeMap.emplace(nodehash, hostList.size() - 1);
		}
	}
	return 0;
}

int Driver::addServerReturn(int id, int status) {
	serverCnt--;
	std::cout << "[DEBUG DRIVER] in addServerReturn: serverCnt: " << serverCnt.load() << std::endl;
	condServer.notify_all();
	return 0;
}

int Driver::removeServer(string &hostname) {
	bool flag;
	{
		lock_guard <mutex> lck(mu);
		flag = enableFlag;
	}
	if (flag) {
		cout << "[DEBUG DRIVER] in removeServer trap disabled." << endl;
		return 1;
	}
	vector <Host> tmp;
	flag = 1;
	{
		lock_guard <mutex> lck(mu);
		for (auto &host : hostList) {
			if (host.hostname == hostname) {
				flag = 0;
			} else {
				tmp.emplace(host);
			}
		}
	}
	if (flag) {
		cout << "[DEBUG DRIVER] in removeServer trap disabled." << endl;
		return 1;
	}
	setEnableFlag(1);
	{
		lock_guard <mutex> lck(mu);
		serverCnt.store(tmp.size());
		unsigned id = opid++;
		for (auto &host : tmp) {
			msgHandler::sendRemoveServer(id, localhost.ip, localhost.port, host.ip, host.port, hostname);
		}
	}
    {
        unique_lock<mutex> lck(mu);
        condServer.wait(lck, [this]() {
        	return serverCnt.load() == 0;
        });
    }
	setEnableFlag(0);
}

int Driver::actRemoveServer(string &hostname) {
	lock_guard <mutex> lck(mu);
	vector <Host> tmp = hostList;
	hostList.clear();
	for (auto &host : tmp) {
		if (host.hostname != hostname) {
			hostList.emplace(host);
		}
	}

	for (int i = 0; i < hostList.size(); i++) {
		auto &host = hostList[i];
		ostringstream buf;
		string tmpstr;
		unsigned nodehash;
		for (int j = 0; j < (NODECOPY); j++) {
			buf.str("");
			buf << host.hostname << "#" << j;
			tmpstr = buf.str();
			nodehash = hash(tmpstr);
			if (!nodeMap.count(nodehash)) {
				nodeMap.emplace(nodehash, i);
			}
		}
	}

	return 0;
}

int Driver::removeServerReturn(int id, int status) {
	serverCnt--;
	std::cout << "[DEBUG DRIVER] in removeServerReturn: serverCnt: " << serverCnt.load() << std::endl;
	condServer.notify_all();
	return 0;
}

int Driver::test() {
	for (auto &host : hostList) {
		cout << "[DEBUG TEST] hostname: " << host.hostname << " ip: " << host.ip << " port: " << host.port << endl;
	}
}

