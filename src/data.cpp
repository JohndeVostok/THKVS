#include "data.h"
#include "msg_handler.hpp"
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
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );
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
    //printf("get_return id:%d status:%s value:%s time_stamp:%lld\n", id, status.c_str(), value.c_str(), time_stamp);
    //return "get_return success";
}
void Data::put_return(int id, string ip, int port, int status)
{
    msgHandler::sendPutRet(id, ip, port, status);
    //printf("put_return id:%d value:%s\n", id, status.c_str());
    //return "put_return success";
}

void Data::run()
{
    string key = "0";
    string value = "0";
    while(true) {
        KeyValue kv = KeyValue(0, key, 0, key, value, 0);
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
            umap[kv.key] = V;
            status = 0;
            put_return(kv.id, kv.ip, kv.port, status);
        }
        if (kv.op == 2)
        {
            int status = 1;
            int time_stamp = 0;
            if (umap.find(kv.key) == umap.end())
                status = 1;
            else
            {
                V = umap[kv.key];
                status = 0;
            }
            get_return(kv.id, kv.ip, kv.port, status, V.value, V.time_stamp);
        }
    }
}