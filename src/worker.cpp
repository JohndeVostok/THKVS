//
// Created by 余欣健 on 2018/8/2.
//

#include "worker.h"

namespace Worker {
    job::job(job_type _type, int _id, int _status)
            : type(_type), id(_id), status(_status) {}
    job::job(job_type _type, int _id, int _status, std::string& _value)
            : type(_type), id(_id), status(_status), value(std::move(_value)) {}
    job::job(job_type _type, int _id, std::string& _key)
            : type(_type), id(_id), key(std::move(_key)) {}
    job::job(job_type _type, int _id, std::string& _key, std::string& _value)
            : type(_type), id(_id), key(std::move(_key)), value(std::move(_value)) {}

    Result::Result(int _id, int _status, std::string& _result)
            : id(_id), status(_status), result(std::move(_result)) {}

    MsgQueue<std::shared_ptr<job> > jobQue;
    MsgQueue<std::shared_ptr<Result> > resQue;
    std::unordered_map<int, int> jobIdMap;
    std::atomic<int> jobId{0};
    std::mutex mu;

    void run() {
        while (true) {
            std::shared_ptr<job> J;
            jobQue.pop(J);
            switch(J->type) {
                case job::j_put: {
                    int driverId = Driver::getInstance()->put(J->key, J->value);
                    {
                        std::lock_guard<std::mutex> lck(mu);
                        jobIdMap.emplace(driverId, J->id);
                    }
                    break;
                }
                case job::j_get: {
                    int driverId = Driver::getInstance()->get(J->key);
                    {
                        std::lock_guard<std::mutex> lck(mu);
                        jobIdMap.emplace(driverId, J->id);
                    }
                    break;
                }
                case job::j_result:{
                    int driverId = J->id;
                    int jId;
                    {
                        std::lock_guard<std::mutex> lck(mu);
                        jId = jobIdMap[driverId];
                        jobIdMap.erase(driverId);
                    }
                    Result R(jId, J->status, J->value);
                    resQue.push(std::make_shared<Result>(R));
                    break;
                }
            }
        }
    }

    int insertGet(std::string key) {
        jobId ++;
        job J(job::j_get, jobId.load(), key);
        jobQue.push(std::make_shared<job>(J));
        return jobId.load();
    }

    int insertPut(std::string key, std::string value) {
        jobId ++;
        job J(job::j_put, jobId.load(), key, value);
        jobQue.push(std::make_shared<job>(J));
        return jobId.load();
    }

    void insertGetResult(int driverId, int status, std::string value) {
        job J(job::j_result, driverId, status, value);
        jobQue.push(std::make_shared<job>(J));
        return ;
    }

    void insertPutResult(int driverId, int status) {
        job J(job::j_result, driverId, status);
        jobQue.push(std::make_shared<job>(J));
        return ;
    }
}
