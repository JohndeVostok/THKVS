#include "btree.h"
#include <iostream>
#include <cstring>

Btree::Btree() {
	file.open(THKVS_DATA_FILENAME, ios::in|ios::out|ios::binary|ios::app);
	if (file.tellp()) {
		//TODO::get data from file.
	} else {
		nodepage = 0;
		nodeoff = PAGESIZE;
		rtid = allocNode();
		Node &rt = nodeList[rtid];
		rt.prev = rtid;
		rt.prev = rtid;
	}
}

Btree::~Btree() {}

int Btree::allocNode() {
	int id = nodeList.size();
	Node node;
	node.leaf = 1;
	if (nodeoff == PAGESIZE) {
		nodepage = allocPage();
		nodeoff = 0;
	}
	node.page = nodepage;
	node.off = nodeoff;
	nodeoff += NODESIZE;
	nodeList.emplace_back(node);
	return id;
}

int Btree::allocEntry() {
	int id = entryPage.size();
	int pageid = allocPage();
	cout << pageid << endl;
	entryPage.emplace_back(pageid);
	return id;
}

int Btree::allocPage() {
	char buf[PAGESIZE];
	memset(buf, 0, sizeof(buf));
	file.seekp(0, ios::end);
	int pageid = file.tellp() >> 12;
	file.write(buf, sizeof(buf));
	cout << pageid << endl;
	return pageid;
}

int Btree::dumpNode(int id) {
	Node &node = nodeList[id];
	char buf[NODESIZE];
	memset(buf, 0, sizeof(buf));
	char *tmp;
	buf[0] = 0;
	buf[1] = 1;
	buf[2] = 0;
	buf[3] = 1;
	tmp = reinterpret_cast <char*>(&id);
	for (int i = 0; i < 4; i++) {
		buf[4 + i] = tmp[i];
	}
	tmp = reinterpret_cast <char*>(&node.prev);
	for (int i = 0; i < 4; i++) {
		buf[8 + i] = tmp[i];
	}
	tmp = reinterpret_cast <char*>(&node.succ);
	for (int i = 0; i < 4; i++) {
		buf[12 + i] = tmp[i];
	}
	int size = node.keys.size();
	tmp = reinterpret_cast <char*>(&size);
	for (int i = 0; i < 4; i++) {
		buf[16 + i] = tmp[i];
	}
	for (int i = 0; i < node.keys.size(); i++) {
		tmp = reinterpret_cast <char*>(&node.keys[i]);
		for (int j = 0; j < 4; j++) {
			buf[32 + 4 * i + j] = tmp[j];
		}
	}
	for (int i = 0; i < node.childs.size(); i++) {
		tmp = reinterpret_cast <char*>(&node.childs[i]);
		for (int j = 0; j < 4; j++) {
			buf[48 + 4 * i + j] = tmp[j];
		}
	}
	int hdoff = (node.page << 12) | node.off;
	file.seekp(hdoff, ios::beg);
	file.write(buf, NODESIZE);
}

Btree::Node Btree::loadNode(int page, int off) {
	int hdoff = (page << 12) | off;
	char buf[NODESIZE];
	file.seekg(hdoff, ios::beg);
	file.read(buf, NODESIZE);
	Node node;
	int *tmp;
	tmp = reinterpret_cast <int*> (buf + 4);
	node.id = *tmp;
	tmp = reinterpret_cast <int*> (buf + 8);
	node.prev = *tmp;
	tmp = reinterpret_cast <int*> (buf + 12);
	node.succ = *tmp;
	tmp = reinterpret_cast <int*> (buf + 16);
	int t = *tmp;
	for (int i = 0; i < t; i++) {
		tmp = reinterpret_cast <int*> (buf + 32 + i * 4);
		node.keys.emplace_back(*tmp);
	}
	for (int i = 0; i < t; i++) {
		tmp = reinterpret_cast <int*> (buf + 48 + i * 4);
		node.childs.emplace_back(*tmp);
	}
	return node;
}

int Btree::dumpEntry(Entry &entry) {
	int hdoff = entryPage[entry.id] << 12;
	cout << hdoff << endl;
	char buf[PAGESIZE];
	memset(buf, 0, sizeof(buf));
	char *tmp;
	buf[0] = 1;
	buf[1] = 0;
	buf[2] = 1;
	buf[3] = 0;
	tmp = reinterpret_cast <char*>(&entry.id);
	for (int i = 0; i < 4; i++) {
		buf[4 + i] = tmp[i];
	}
	tmp = reinterpret_cast <char*>(&entry.timestamp);
	for (int i = 0; i < 8; i++) {
		buf[8 + i] = tmp[i];
	}
	cout << "tmp: ";
	for (int i = 0; i < 8; i++) cout << int(tmp[i]) << " ";
	cout << endl;
	int length;
	length = entry.key.length();
	tmp = reinterpret_cast <char*>(&length);
	for (int i = 0; i < 4; i++) {
		buf[16 + i] = tmp[i];
	}
	length = entry.value.length();
	tmp = reinterpret_cast <char*>(&length);
	for (int i = 0; i < 4; i++) {
		buf[20 + i] = tmp[i];
	}
	for (int i = 0; i < entry.key.length(); i++) {
		buf[32 + i] = entry.key[i];
	}
	for (int i = 0; i < entry.value.length(); i++) {
		buf[64 + i] = entry.value[i];
	}
	cout << "buf: ";
	for (int i = 0; i < 24; i++) cout << int(buf[i]);
	cout << endl;
	file.seekp(hdoff);
	cout << file.tellp() << endl;
	file.write(buf, PAGESIZE);
}

Btree::Entry Btree::loadEntry(int page) {
	cout << "!!" << page << endl;
	int hdoff = page << 12;
	cout << hdoff << endl;
	char buf[PAGESIZE];
	file.seekg(hdoff, ios::beg);
	file.read(buf, PAGESIZE);
	cout << "buf: ";
	for (int i = 0; i < 24; i++) cout << int(buf[i]);
	cout << endl;
	Entry entry;
	int *tmp;
	tmp = reinterpret_cast <int*> (buf + 4);
	entry.id = *tmp;
	long long *ltmp;
	ltmp = reinterpret_cast <long long*> (buf + 8);
	entry.timestamp = *ltmp;
	cout << *ltmp << endl;
	tmp = reinterpret_cast <int*> (buf + 16);
	int kl = *tmp;
	cout << "kl" << kl << endl;
	tmp = reinterpret_cast <int*> (buf + 20);
	int vl = *tmp;
	cout << "vl" << vl << endl;
	entry.key = "";
	for (int i = 0; i < kl; i++) {
		entry.key = entry.key + buf[32 + i];
	}
	entry.value = "";
	for (int i = 0; i < vl; i++) {
		entry.value = entry.value + buf[64 + i];
	}
	return entry;
}

int Btree::put(unsigned key, Entry &entry) {
	entry.id == allocEntry();
}
/*int Btree::split(int id) {
	int id1 = allocNode(), id2 = allocNode();
	Node &node0 = nodeList[id];
	Node &node1 = nodeList[id1];
	Node &node2 = nodeList[id2];
	bool flag = node0.leaf;
	node1.leaf = flag;
	node2.leaf = flag;
	node0.leaf = 0;
	if (flag) {
		node1.prev = node0.prev;
		node1.succ = id2;
		node2.prev = id1;
		node2.succ = node0.succ;
	}
}

int Btree::insert(unsigned key, int id) {
}

int Btree::insert(unsigned key) {
	Node &rt = nodeList[rt];
	if (rt.keys.size() == (BMIN << 1)) {
		split(rt);
	}
}
*/
void Btree::test() {
//	Entry test.
	int id = allocEntry();
	Entry entry;
	entry.id = id;
	entry.timestamp = 1111;
	entry.key = "sb";
	entry.value = "cnm";
	dumpEntry(entry);
	Entry e = loadEntry(entryPage[id]);
	cout << e.id << " " << e.timestamp << " " << e.key << " " << e.value << endl;
/*
	Node test.
	int id = allocNode();
	Node &node = nodeList[id];
	node.prev = 8;
	node.succ = 15;
	node.keys.emplace_back(99);
	node.childs.emplace_back(37);
	dumpNode(id);
	Node p = loadNode(0, 0);
	cout << node.prev << " " << node.succ << " " << node.keys.size() << " " << node.keys[0] << endl;
*/
}
