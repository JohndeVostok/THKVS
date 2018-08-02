//
// Created by 余欣健 on 2018/8/2.
//

#ifndef THKVS_WORKER_H
#define THKVS_WORKER_H

#include <unordered_map>
#include "network/message.hpp"
#include "network/msg_queue.hpp"
#include "driver.h"

namespace Worker {

    class job {
    public:
        enum job_type {
            j_put = 1,
            j_get = 2,
            j_result = 3
        };
        job_type type; // result, job
        int id;
        int status;
        std::string key, value;
        job(job_type _type, int _id, int _status);
        job(job_type _type, int _id, int _status, std::string& _value);
        job(job_type _type, int _id, std::string& _key);
        job(job_type _type, int _id, std::string& _key, std::string& _value);
    };

    class Result {
    public:
        int id, status;
        std::string result;
        Result(int _id, int _status, std::string& _result);
    };

    extern MsgQueue<std::shared_ptr<job> > jobQue;
    extern MsgQueue<std::shared_ptr<Result> > resQue;
    extern std::unordered_map<int, int> jobIdMap;
    extern std::atomic<int> jobId, testCnt;
    extern std::condition_variable testCond;
    extern std::mutex mu, testMu;
	extern bool displayFlag;

    void run();

    int insertGet(std::string key);

    int insertPut(std::string key, std::string value);

    void insertGetResult(int driverId, int status, std::string value);

    void insertPutResult(int driverId, int status);

	void setDisplay();

}


#endif //THKVS_WORKER_H
