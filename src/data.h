#ifndef DATA_H
#define DATA_H

#include "network/msg_queue.hpp"
#include "crc.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "msg_handler.hpp"

using namespace std;


class Data {
public:
    Data(Data const&) = delete;
    void operator = (Data const&) = delete;
    static Data* getInstance()  {
        static Data _inst;
        return &_inst;
    }
    class KeyValue
    {
    public:
        int id, op, port, remotesrcport, remotedestport;
        unsigned int begin, end;
        string ip, remotesrcip, remotedestip;
        string key, value;
        list<string> keyVec, valueVec;
        KeyValue(int _id, string _ip, int _port, string _key, string _value, int _op)
        {
            op = _op;
            ip = _ip;
            port = _port;
            id = _id;
            key = _key;
            value = _value;
        }
        //recvMoveData
        KeyValue(int _id, string &_srcip, int _srcport, list<string> _keyVec, list<string> _valueVec, int _op)
        {
            id = _id;
            ip = _srcip;
            port = _srcport;
            keyVec = _keyVec;
            valueVec = _valueVec;
            op = _op;
        }
        //getMoveData
        KeyValue(int _id, unsigned _begin, unsigned _end, string &_srcip, int _srcport, string &_remotesrcip, int _remotesrcport, string &_remotedestip, int _remotedestport, int _op)
        {
            id = _id;
            begin = _begin;
            end = _end;
            ip = _srcip;
            port = _srcport;
            remotesrcip = _remotesrcip;
            remotesrcport = _remotesrcport;
            remotedestip = _remotedestip;
            remotedestport = _remotedestport;
            op = _op;
        }
    };

    struct Value
    {
        int id;
        unsigned int key;
        string value;
        long long time_stamp;
        friend bool operator < (Value a, Value b)
        {
            return a.key < b.key;
        }
    };

    struct Hashlist
    {
        unsigned int begin, end;
        int cnt;
    };

    mutex mu;

    vector<Hashlist> hl;

    string conkey(unsigned int key);
    string conkey2(long long key);
    void get(int id, string ip, int port, string key);
    void put(int id, string ip, int port, string key, string value);
    void get_return(int id, string ip, int port, int status, string value, long long time_stamp);
    void put_return(int id, string ip, int port, int status);
    void run();

    //op 3
    void getMoveData(int id, unsigned int begin, unsigned int end, string srcip, int srcport, string remotesrcip, int remotesrcport, string remotedestip, int remotedestport, bool remove);
    void getMoveData_return(int id, string remotesrcip, int remotesrcport, string remotedestip, int remotedestport, list<string> keyVec, list<string> valueVec);

    //op4
    void recvMoveData(int id, string srcip, int srcport, list<string> keyVec, list<string> valueVec);
    void recvMoveData_return(int id, string srcip, int srcport, int status);

    //op5
    void moveDataReturn(int id, int status);
    void moveDataReturn_return(int id, string ip, int port, int status);


    long long tolonglong(string key);
    unsigned int toint(string key);

    map<int, string> mip;
    map<int, int> mpo;

private:
    unordered_map<string, Value> umap;
    MsgQueue<KeyValue> que;

    Data() {
        std::cout << "[DEBUG DATA] in constructor: Hello World" << std::endl;
    };
    ~Data() {};
};

#endif