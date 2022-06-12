//
// Created by zeb on 06/06/2022.
//

#ifndef MINIMUMMEANCYCLE_THREADPOOL_HPP
#define MINIMUMMEANCYCLE_THREADPOOL_HPP

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "CoTask.hpp"

namespace ps_framework {

    class ThreadPool {
    public:
        ThreadPool(int numThreads);
        ThreadPool();
        ~ThreadPool();
        void AddJob(coroTaskVoid *);
        void shutdown();
        void waitUntilFinished();
    private:
        void loopFunc();
        std::queue<coroTaskVoid *> jobQueue;
        std::vector<std::thread> threadVec;
        std::mutex qMutex;
        std::condition_variable cv_job;
        std::condition_variable cv_finished;
        uint workingThreads;
        bool terminate;
        bool stopped;

    };

}

#endif //MINIMUMMEANCYCLE_THREADPOOL_HPP
