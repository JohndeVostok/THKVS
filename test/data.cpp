#include "data.h"
#include <string>
#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>    // for gettimeofday()
#include <iostream>
#include <fstream>

using namespace std;

long long getTime()
{
    struct timeval now;
    gettimeofday(&now, NULL);
	long long real = now.tv_sec * 1000000 + now.tv_usec;
    time_t timep;
    return real;
}

void Data::put(int id, string ip, int port, string key, string value)
{
	que.push(KeyValue(id, ip, port, key, value, 1));
	return;
}

void Data::get(int id, string ip, int port, string key)
{
	string value = "haha";
	que.push(KeyValue(id, ip, port, key, value, 2));
	return;
}

void Data::get_return(int id, string ip, int port, int status, string value, long long time_stamp)
{
	//msgHandler::sendGetRet(id, ip, port, status, value, time_stamp);
	printf("get_return id:%d status:%d value:%s time_stamp:%lld\n", id, status, value.c_str(), time_stamp);
	//return "get_return success";
}
void Data::put_return(int id, string ip, int port, int status)
{
	//msgHandler::sendPutRet(id, ip, port, status);
	printf("put_return id:%d status:%d\n", id, status);
	//return "put_return success";
}

string Data::conkey(unsigned int key)
{
	string ret = "";
	if (key == 0)
		return "0";
	while (key > 0)
	{
		ret = char(key % 10 + 48) + ret;
		key = key / 10;
	}
	return ret;
}

string Data::conkey2(long long key)
{
	string ret = "";
	while (key > 0)
	{
		ret = char(key % 10 + 48) + ret;
		key = key / 10;
	}
	return ret;
}

long long Data::tolonglong(string key)
{
	int length = key.size();
	long long time_stamp = 0;
	for (int i = 0; i <= length - 1; i++)
	{
		time_stamp = time_stamp * 10 + key[i] - '0';
	}
	return time_stamp;
}

unsigned int Data::toint(string key)
{
	int length = key.size();
	int time_stamp = 0;
	for (int i = 0; i <= length - 1; i++)
	{
		time_stamp = time_stamp * 10 + key[i] - '0';
	}
	return time_stamp;
}


//hashlist
//hl_cnt
//begin end, cnt
//

//{{{
void Data::run()
{
	string str = "rm *.txt";
	system(str.c_str());
	Hashlist tmp;
	tmp.begin = 0;
	tmp.end = 4294967295;
	tmp.cnt = 0;
	hl.push_back(tmp);

	ofstream output_1;
	string file_name_1 = conkey(tmp.begin) + "_" + conkey(tmp.end) + ".txt"; // 用文件编号构造文件名
	output_1.open(file_name_1, ios::out);

	string key_1 = "0";
	string value_1 = "0";

	while(true) 
	{
		printf("running\n");
		KeyValue kv = KeyValue(0, key_1, 0, key_1, value_1, 0);
		/*
		if (que.empty() == true)
		{
			continue;
		}
		*/
		que.pop(kv);
		//put 
		Value V;
		puts("");
		V.id = kv.id;
		V.value = kv.value;
		V.time_stamp = getTime();


		//key:kv.key(string), kv.value(string), V.time_stamp(long long)
		//key:"fuck you", "asshole", "294719579252"
		//hl[0].begin:1 hl[0].end:2147483647 hl[0].cnt = 0
		//
		if (kv.op == 1)
		{
			printf("now kv.op == 1\n");
			//0 means success
			int status = 1;

			int sz = hl.size();
			printf("sz:%d\n", sz);
			unsigned int hash_value = getCRC(kv.key);
			printf("hash_value:%u\n", hash_value);

			for (int i = 0; i < sz; i++)
			{

				//means the hash value is in this range
				if (not (hl[i].begin <= hash_value && hash_value <= hl[i].end))
					continue;

				ifstream input;
				string file_name = conkey(hl[i].begin) + "_" + conkey(hl[i].end) + ".txt";

				printf("file_name:%s\n", file_name.c_str());
				input.open(file_name, ios::in);
				//means when reading, there is an error occured
				if (!input)
				{
					status = 1;
					put_return(kv.id, kv.ip, kv.port, status);
					break;
				}
				//change 1 to block size
				//means we need to divide
				bool flag = false;
				Value tmp[10000];

				for (int j = 0; j < hl[i].cnt; j++)
				{
					input >> tmp[j].key >> tmp[j].value >> tmp[j].time_stamp;
					if (tmp[j].key == hash_value)
					{
						tmp[j].value = kv.value;
						tmp[j].time_stamp = V.time_stamp;
						flag = true;
						break;
					}
				}
				//key:kv.key(string), kv.value(string), V.time_stamp(long long)
				// means we update the key value;

				//now the flag == false means that it is a new key.
				if (flag == false)
				{
					printf("flag == false!!\n");
					tmp[hl[i].cnt].key = hash_value;
					tmp[hl[i].cnt].value = kv.value;
					tmp[hl[i].cnt].time_stamp = V.time_stamp;
					hl[i].cnt += 1;
				}
				
				sort(tmp, tmp + hl[i].cnt);
				printf("kv.value:%s\n", kv.value.c_str());

				for (int j = 0; j < hl[i].cnt; j++)
					printf("key:%u value:%s time_stamp:%lld\n", tmp[j].key, tmp[j].value.c_str(), tmp[j].time_stamp);

				int cut = hl[i].cnt / 2;

				//means now we do not need to divide it
				if (hl[i].cnt <= 4)
				{
					ofstream output_1;
					string file_name_1 = conkey(hl[i].begin) + "_" + conkey(hl[i].end) + ".txt"; // 用文件编号构造文件名
					output_1.open(file_name_1, ios::out);

					for (int j = 0; j < hl[i].cnt; j++)
						output_1 << tmp[j].key << " " << tmp[j].value << " " << tmp[j].time_stamp << endl;
					break;
					//success
					put_return(kv.id, kv.ip, kv.port, 0);
				}
				//means we need to divide it into 2 parts
				else
				{
					//add a new one or .. update one

					ofstream output_1;
					//from "0" to "cut"
					string file_name_1 = conkey(hl[i].begin) + "_" + conkey(tmp[cut].key) + ".txt"; // 用文件编号构造文件名
					output_1.open(file_name_1, ios::out);
					for (int j = 0; j < cut; j++)
						output_1 << tmp[j].key << " " << tmp[j].value << " " << tmp[j].time_stamp << endl;

					ofstream output_2;
					string file_name_2 = conkey(tmp[cut].key) + "_" + conkey(hl[i].end) + ".txt"; // 用文件编号构造文件名
					output_2.open(file_name_2, ios::out);
					//from "cut" to "hl[i].cnt"
					for (int j = cut; j < hl[i].cnt; j++)
						output_2 << tmp[j].key << " " << tmp[j].value << " " << tmp[j].time_stamp << endl;

					string file_name = "rm " + conkey(hl[i].begin) + "_" + conkey(hl[i].end) + ".txt";

					system(file_name.c_str());

					//hl[i]:
					//
					printf("dividing!!! hash_value:%u begin:%u end:%u\n", hash_value, hl[i].begin, hl[i].end);
					
					Hashlist t;
					t.begin = tmp[cut].key;
					t.end = hl[i].end;
					t.cnt = hl[i].cnt - cut;
					hl.push_back(t);
					
					hl[i].begin = hl[i].begin;
					hl[i].end = tmp[cut].key;
					hl[i].cnt = cut;




					//success
					put_return(kv.id, kv.ip, kv.port, 0);
				}
			}

		}

		if (kv.op == 2) //get opreation
		{
			printf("======kv.op == 2 =====\n");
			int status = 1;
			string key, value, time_stamp;

			int sz = hl.size();
			printf("sz:%d\n", sz);
			bool flag = false;

			unsigned int hash_value = getCRC(kv.key);

			for (int i = 0; i < sz; i++)
			{
				printf("hash_value:%u begin:%u end:%u\n", hash_value, hl[i].begin, hl[i].end);
				//means the hash value is in this range
				if (not (hl[i].begin <= hash_value && hash_value <= hl[i].end))
					continue;

				ifstream input;
				string file_name = conkey(hl[i].begin) + "_" + conkey(hl[i].end) + ".txt";
				input.open(file_name, ios::in);
				if (!input)
				{
					status = 1;
					get_return(kv.id, kv.ip, kv.port, status, value, tolonglong(time_stamp));
					flag = true;
					break;
				}

				for (int j = 0; j < hl[i].cnt; j++)
				{
					input >> key >> value >> time_stamp;
					//means found
					printf("str_key:%s\n", key.c_str());
					if (toint(key) == hash_value)
					{
						//meas found
						printf("FOUNDDDDDD!!!! kv.key%s\n", kv.key.c_str());
						flag = true;
						status = 0;
						get_return(kv.id, kv.ip, kv.port, status, value, tolonglong(time_stamp));
						break;
					}
				}
			}

			if (flag == true) continue;

			status = 1;
			printf("status:%d value:%s\n", status, value.c_str());
			get_return(kv.id, kv.ip, kv.port, status, value, tolonglong(time_stamp));
		}
	}
}
//}}}
