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
	if (iter == nodeMap.end()) {
		iter = nodeMap.begin();
	}
	while (hosts.size() < THKVS_N) {
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
		iter++;
		if (iter == nodeMap.end()) {
			iter = nodeMap.begin();
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
		msgHandler::sendPut(id, localhost.ip, localhost.port, host.ip, host.port, key, value);
	}
}

int Driver::putReturn(int id, int status) {

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
	cout << "[DEBUG DRIVER] in putFinish id: " << id << " status: " << status << endl;
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
	vector <int> hosts;
	getHosts(key, hosts);
	for (int hostIdx : hosts) {
		Host& host = hostList[hostIdx];
		msgHandler::sendGet(id, localhost.ip, localhost.port, host.ip, host.port, key);
	}
}

int Driver::getReturn(int id, int status, long long timestamp, string &value) {
	int flag = 0;
	string str;
	{
		lock_guard <mutex> lck(mu);
		if (entries.find(id) == entries.end()) {
			flag = 1;
		}
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
	cout << "[DEBUG DRIVER] in getFinish id: " << id << " status: " << status << " value: " << value << endl;
}

//Fault tolerance

int Driver::setEnableFlag(bool flag) {
	unsigned id;
	{
		lock_guard <mutex> lck(mu);
		id = enableFlagEntry.id = opid++;
		enableFlagEntry.cnt = 0;
	}
	enableCnt.store(hostList.size());
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
}

int Driver::setEnableFlagReturn(int id, int status) {
	enableCnt--;

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
		return 1;
	}

	vector <pair <unsigned, int>> pre, suc;
	ostringstream buf;
	string tmpstr;
	unsigned nodehash;
	moveCnt.store(0);
	map <unsigned, int> tmpMap = nodeMap;
	for (int i = 0; i < NODECOPY; i++) {
		buf.str("");
		buf << hostname << "#" << i;
		tmpstr = buf.str();
		nodehash = hash(tmpstr);
		if (!tmpMap.count(nodehash)) {
			tmpMap.emplace(nodehash, hostList.size());
		}
	}

	vector <int> succ;
	unsigned hashbegin, hashend;
	for (auto iter = tmpMap.begin(); iter != tmpMap.end(); iter++) {
		succ.clear();
		hashbegin = iter->first;
		bool curFlag = 0;
		auto jter = iter;
		jter++;
		if (jter == tmpMap.end()) {
			jter = tmpMap.begin();
		}
		hashend = jter->first;
		while (succ.size() < THKVS_N + curFlag) {
			bool uniqueFlag = 0;
			if (jter->second == hostList.size()) {
				curFlag = 1;
			}
			for (auto id : succ) {
				if (jter->second == id) {
					uniqueFlag = 1;
				}
			}
			if (!uniqueFlag) {
				succ.emplace_back(jter->second);
			}
			jter++;
			if (jter == tmpMap.end()) {
				jter = tmpMap.begin();
			}
		}
		
		if (succ.size() > THKVS_N) {
			moveCnt++;
			opid++;
			unsigned id = opid.load();
			auto &srchost = hostList[succ.back()];
			cout << "[DEBUG DRIVER] in addServer send move msg: id: " << id << " srcport: " << srchost.port << " destport: " << port << " begin: " << hashbegin << " end: " << hashend << endl;
			msgHandler::sendMove(id, localhost.ip, localhost.port, srchost.ip, srchost.port, ip, port, hashbegin, hashend);
		}
	}

    {
        unique_lock<mutex> lck(mu);
        condMove.wait(lck, [this](){
        	return moveCnt.load() == 0;
        });
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
    {
        unique_lock<mutex> lck(mu);
        condServer.wait(lck, [this](){
        	return serverCnt.load() == 0;
        });
    }
	setEnableFlag(0);
	cout << "[DEBUG DRIVER] in addServer" << endl;
	return 0;
}

int Driver::actAddServer(string &hostname, string &ip, int port) {
	lock_guard <mutex> lck(mu);
	Host tmp;
	tmp.hostname = hostname;
	tmp.ip = ip;
	tmp.port = port;
	hostList.emplace_back(tmp);

	ostringstream buf;
	string tmpstr;
	unsigned nodehash;
	for (int i = 0; i < NODECOPY; i++) {
		buf.str("");
		buf << hostname << "#" << i;
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
		return 1;
	}

	flag = 1;
	Host chost;
	for (auto &host : hostList) {
		if (host.hostname == hostname) {
			flag = 0;
			chost = host;
		}
	}
	if (flag) {
		return 1;
	}

	vector <pair <unsigned, int>> pre, suc;
	ostringstream buf;
	string tmpstr;
	unsigned nodehash;
	moveCnt.store(0);
	for (int i = 0; i < NODECOPY; i++) {
		buf.str("");
		buf << hostname << "#" << i;
		tmpstr = buf.str();
		nodehash = hash(tmpstr);
		if (!nodeMap.count(nodehash)) {
			moveCnt++;
			pre.clear();
			suc.clear();
			auto iter = nodeMap.upper_bound(nodehash);
			if (iter == nodeMap.begin()) {
				iter = nodeMap.end();
			}
			iter--;
			while (pre.size() < THKVS_N) {
				flag = 0;
				for (auto &node : pre) {
					if (iter->second == node.second) {
						flag = 1;
					}
				}
				if (!flag) {
					pre.emplace_back(iter->first, iter->second);
				}
                iter--;
				if (iter == nodeMap.begin()) {
					iter = nodeMap.end();
				}
			}
			iter = nodeMap.upper_bound(nodehash);
			if (iter == nodeMap.end()) {
				iter = nodeMap.begin();
			}
			while (suc.size() < THKVS_N) {
				flag = 0;
				for (auto &node : suc) {
					if (iter->second == node.second) {
						flag = 1;
					}
				}
				if (!flag) {
					suc.emplace_back(iter->first, iter->second);
				}
				iter++;
				if (iter == nodeMap.end()) {
					iter = nodeMap.begin();
				}
			}
			unsigned hashend = nodehash, hashbegin;
			Host srchost = chost;
			for (int i = 0; i < THKVS_N; i++) {
				opid++;
				unsigned id = opid.load();
				hashbegin = pre[i].first;
				auto &desthost = hostList[suc[THKVS_N - i - 1].second];
				cout << "[DEBUG DRIVER] in removeServer send move msg: id: " << id << " srcport: " << srchost.port << " destport: " << desthost.port << " begin: " << hashbegin << " end: " << hashend << endl;
				msgHandler::sendMove(id, localhost.ip, localhost.port, srchost.ip, srchost.port, desthost.ip, desthost.port, hashbegin, hashend);
				hashend = hashbegin;
				srchost = hostList[pre[i].second];
			}
		}
	}

    {
        unique_lock<mutex> lck(mu);
        condMove.wait(lck, [this](){
        	return moveCnt.load() == 0;
        });
    }

	setEnableFlag(1);
	{
		lock_guard <mutex> lck(mu);
		serverCnt.store(hostList.size() - 1);
		unsigned id = opid++;
		for (auto &host : hostList) {
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
	cout << "[DEBUG DRIVER] in removeServer" << endl;
}

int Driver::actRemoveServer(string &hostname) {
	lock_guard <mutex> lck(mu);
	vector <Host> tmp = hostList;
	hostList.clear();
	for (auto &host : tmp) {
		if (host.hostname != hostname) {
			hostList.emplace_back(host);
		}
	}
    nodeMap.clear();
	for (int i = 0; i < hostList.size(); i++) {
		auto &host = hostList[i];
		ostringstream buf;
		string tmpstr;
		unsigned nodehash;
		for (int j = 0; j < NODECOPY; j++) {
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
	condServer.notify_all();
	return 0;
}

int Driver::moveReturn(int id, int status) {
	moveCnt--;
	condMove.notify_all();
	return 0;
}

int Driver::test() {
	for (auto &host : hostList) {
		cout << "[DEBUG DRIVER] at test: hostname: " << host.hostname << " ip: " << host.ip << " port: " << host.port << endl;
	}
	cout << "[DEBUG DRIVER] at test: nodeMap:" << endl;
	for (auto iter = nodeMap.begin(); iter != nodeMap.end(); iter++) {
		cout << "hash: " << iter->first << "id: " << iter->second << endl;
	}
}

