//
// Created by zeb on 14/04/2022.
//

#include "ThreadPool.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>

// This implementation based on the approach given in the
// answer in https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
// and waitUntillFinshed mechanism is borrowed from:
// https://stackoverflow.com/questions/23896421/efficiently-waiting-for-all-tasks-in-a-threadpool-to-finish


// Initilallize pool with specified number of threads
ps_framework::ThreadPool::ThreadPool(int numThreads) {
    terminate = false;
    workingThreads = 0;
    threadVec.reserve(numThreads);
    for (int i = 0; i<numThreads; i++){
        threadVec.push_back(std::thread([this]{this->loopFunc();}));
    }
}

// If no number of threads is specified then the
// maximal number of threads the system supports is selected
ps_framework::ThreadPool::ThreadPool() {
    stopped = false;
    workingThreads = 0;
    int numThreads = std::thread::hardware_concurrency();
    threadVec.reserve(numThreads);
    for (int i = 0; i<numThreads; i++){
//        threadVec.push_back(std::thread([this]{this->loopFunc();}));
        threadVec.push_back(std::thread(&ThreadPool::loopFunc,this));
    }
}

void ps_framework::ThreadPool::loopFunc() {
    while (true) {
        ps_framework::co_task_void* job;
        // Lock mutex or wait until unlocked
        std::unique_lock<std::mutex> lock(qMutex);
        cv_job.wait(lock, [this](){
            return !jobQueue.empty() || terminate;
        });
        // Increment workingThreads counter
        ++workingThreads;
        // Pop a job:)
        job = jobQueue.front();
        jobQueue.pop();
        // Unlock mutex
        lock.unlock();
        cv_job.notify_one();
        // Run coroutine until suspended
        job->resume();
        // Lock mutex or wait until unlocked
//        cv_finished.wait(lock);
        lock.lock();
        --workingThreads;
        // Unlock mutex
        lock.unlock();
        cv_finished.notify_one();
    }
}

// Pauses execution of caller until all threads have worked all jobs in queue
void ps_framework::ThreadPool::waitUntilFinished() {
    std::unique_lock<std::mutex> lc (qMutex);
    cv_finished.wait(lc, [this] {return jobQueue.empty() && (workingThreads == 0);});
}

// Add job to job queue
void ps_framework::ThreadPool::AddJob(ps_framework::co_task_void* job){
    {
        std::unique_lock<std::mutex> lock(qMutex);
        jobQueue.push(job);
    }
    cv_job.notify_one();
}

// Taken from https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
void ps_framework::ThreadPool::shutdown() {
    {
        {
            std::unique_lock<std::mutex> lock(qMutex);
            terminate = true; // use this flag in condition.wait
        }

        cv_job.notify_all(); // wake up all threads.

        // Join all threads.
        for (std::thread &th : threadVec)
        {
            th.join();
        }

        threadVec.clear();
        stopped = true; // use this flag in destructor, if not set, call shutdown()
    }
}

ps_framework::ThreadPool::~ThreadPool() {
    if (!stopped)
        shutdown();
}
