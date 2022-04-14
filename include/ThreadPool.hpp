//
// Created by zeb on 14/04/2022.
//
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "co_task.hpp"

#ifndef MEDIANOFLINES_THREADPOOL_H
#define MEDIANOFLINES_THREADPOOL_H


namespace ps_framework {

    class ThreadPool {
    public:
        ThreadPool(int numThreads);
        ThreadPool();
        ~ThreadPool();
        void AddJob(ps_framework::co_task_void *);
        void shutdown();
        void waitUntilFinished();
    private:
        void loopFunc();
        std::queue<ps_framework::co_task_void *> jobQueue;
        int numberOfThreads;
        std::vector<std::thread> threadVec;
        std::mutex qMutex;
        std::condition_variable cv_job;
        std::condition_variable cv_finished;
        uint workingThreads;
        bool terminate;
        bool stopped;

    };

}


#endif //MEDIANOFLINES_THREADPOOL_H
