#include "data.h"
#include <string>
#include <cstdio>
#include <time.h>
#include <sys/time.h>    // for gettimeofday()

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
	msgHandler::sendGetRet(id, ip, port, status, value, time_stamp);
	//printf("get_return id:%d status:%d value:%s time_stamp:%lld\n", id, status, value.c_str(), time_stamp);
	//return "get_return success";
}
void Data::put_return(int id, string ip, int port, int status)
{
	msgHandler::sendPutRet(id, ip, port, status);
	//printf("put_return id:%d status:%d\n", id, status);
	//return "put_return success";
}

string Data::conkey(int key)
{
	string ret = "";
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

long long tolonglong(string key)
{
	int length = key.size();
	long long time_stamp = 0;
	for (int i = 0; i <= length - 1; i++)
	{
		time_stamp = time_stamp * 10 + key[i] - '0';
	}
	return time_stamp;
}

void Data::run()
{
	string key = "0";
	string value = "0";
	while(true) {
		//printf("running\n");
		KeyValue kv = KeyValue(0, key, 0, key, value, 0);
		/*
		if (que.empty() == true)
		{
			continue;
		}
		*/
		que.pop(kv);
		//put 
		Value V;
		V.id = kv.id;
		V.value = kv.value;
		V.time_stamp = getTime();
		if (kv.op == 1)
		{
			//0 means success
			int status = 1;
			//umap[kv.key] = V;
			//printf("kv.key:%s kv.value:%s\n", kv.key.c_str(), kv.value.c_str());
			hs.put(kv.key, kv.value + conkey2(V.time_stamp));
			//printf("whole_setence:%s\n", (kv.value + conkey2(V.time_stamp)).c_str());
			status = 0;
			put_return(kv.id, kv.ip, kv.port, status);
		}

		if (kv.op == 2)
		{
			int status = 1;

			string ret = hs.get(kv.key);
			//printf("!!!RET!!!!:%s\n", ret.c_str());
			string value = "";
			string time_stamp = "";

			int length = ret.size();
			int cnt = 0;
			for (int i = length - 1; i >= 0; i--)
			{
				cnt += 1;
				if (cnt >= 17)
					value = ret[i] + value;
				else
					time_stamp = ret[i] + time_stamp;
			}

			//printf("value:%s\n", value.c_str());
			//printf("time_stamp:%s\n", time_stamp.c_str());

			if (value == "")
				status = 1;
			else
				status = 0;

			get_return(kv.id, kv.ip, kv.port, status, value, tolonglong(time_stamp));
		}
	}
}
