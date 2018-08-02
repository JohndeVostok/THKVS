#ifndef THKVS_BTREE_H
#define THKVS_BTREE_H

#include <fstream>
#include <string>
#include <vector>

using namespace std;

class Btree {
private:
	struct Node {
		int id, prev, succ, page, off;
		bool leaf;
		vector <unsigned> keys;
		vector <int> childs;
	};

	struct Entry {
		int id;
		long long timestamp;
		string key, value;
	};

	const string THKVS_DATA_FILENAME = "data";
	const int PAGESIZE = 4096;
	const int NODESIZE = 64;
	const int KEYSIZE = 32;
	const int VALUESIZE = 4032;
	const int BMIN = 2;
	const int BMAX = 4;

	//file handle
	int nodepage, nodeoff, head;

	fstream file;
	vector <int> entryPage;
	vector <Node> nodeList;
	int rtid;

	int allocNode();
	int allocEntry();
	int allocPage();
	int dumpNode(int id);
	Node loadNode(int page, int off);
	int dumpEntry(Entry &entry);
	Entry loadEntry(int page);
public:
	Btree();
	~Btree();
	int put(unsigned key, Entry &entry);
	int get(unsigned key, Entry &entry);
	int collect(unsigned begin, unsigned end, vector <Entry> &entries);
	int remove(unsigned begin, unsigned end);
	void test();
};

#endif
