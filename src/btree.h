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

	const string THKVS_DATA_FILENAME = "data";
	const int PAGESIZE = 4096;
	const int NODESIZE = 64;
	const int KEYSIZE = 32;
	const int VALUESIZE = 4032;
	const int BMIN = 2;
	const int BMAX = 4;

	//file handle
	int nodepage, nodeoff;

	fstream file;
	vector <Node> nodeList;
	int rtid;

	int allocNode();
	int allocPage(int pageNum = 1);
	int dumpNode(int id);
	Node loadNode(int page, int off);
public:
	Btree();
	~Btree();
	int insert(unsigned key);
	void test();
};

#endif
