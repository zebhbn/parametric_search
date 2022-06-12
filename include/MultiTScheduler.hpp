//
// Created by zeb on 06/06/2022.
//

#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ThreadPool.hpp"
#include <mutex>
#include <condition_variable>

#ifndef MINIMUMMEANCYCLE_MULTITSCHEDULER_HPP
#define MINIMUMMEANCYCLE_MULTITSCHEDULER_HPP

namespace ps_framework {
    class MultiTScheduler : public Scheduler {
    public:
        MultiTScheduler();
        MultiTScheduler(int numThreads);
        void run();
        void spawnIndependent(coroTaskVoid *task);
        void spawnIndependentIntermediate(coroTaskVoid *task);
        schedulerAwaitable spawnDependent(coroTaskVoid *task);
        schedulerAwaitable spawnDependentIntermediate(coroTaskVoid *task);
        void spawnHandler(std::coroutine_handle<promise_type_void> *handler);

    protected:
        void runHandlers();
        coroTaskVoid runTask(
                coroTaskVoid *task,
                std::map<int ,coroTaskVoid*> *map,
                std::mutex *mapMutex,
                std::condition_variable *mapCV
                );
        int initNewIdCount();
        // Added for saf
        std::mutex idCounterMutex;
        std::condition_variable cv_idCounter;
        std::mutex pendingQueueMutex;
        std::condition_variable cvPending;
        std::mutex pendingInterQueueMutex;
        std::condition_variable cvPendingInter;
        std::mutex handlerQueueMutex;
        std::condition_variable cvHandler;
        void incrementId(int id);

    private:
        ThreadPool* threadPool;
    };
}

void ps_framework::MultiTScheduler::incrementId(int id) {
    // Increment id dependent counter
    std::unique_lock lock(idCounterMutex);
    cv_idCounter.wait(lock, [this]{return true;});
    idCounter[id]++;
    // Unlock
    lock.unlock();
    cv_idCounter.notify_one();
}

ps_framework::MultiTScheduler::MultiTScheduler(int numThreads) {
    threadPool = new ThreadPool(numThreads);
}

ps_framework::MultiTScheduler::MultiTScheduler() {
    threadPool = new ThreadPool();
}

int ps_framework::MultiTScheduler::initNewIdCount() {
    auto id = getNewId();
    // Lock or wait until unlocked
    std::unique_lock lock(idCounterMutex);
    cv_idCounter.wait(lock, [this]{return true;});
    idCounter[id] = 0;
    // Unlock
    lock.unlock();
    cv_idCounter.notify_one();
    return id;
}

void ps_framework::MultiTScheduler::spawnIndependent(coroTaskVoid *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Wrap the task in a run task coroutine
    // and add to thread pool
    auto job = new coroTaskVoid(runTask(task, &pendingTasks, &pendingQueueMutex, &cvPending));
    threadPool->AddJob(job);
}

void ps_framework::MultiTScheduler::spawnIndependentIntermediate(coroTaskVoid *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    activeIntermediateTasks.push(task);
}


ps_framework::Scheduler::schedulerAwaitable ps_framework::MultiTScheduler::spawnDependent(
        coroTaskVoid *task) {
    // This makes sure that we first spawn the job afterwards
    auto spawnJob = new coroTaskVoid(
            [](auto task, auto scheduler) -> coroTaskVoid {
                scheduler->spawnIndependent(task);
                co_return;
            }(task, this)
            );
    return schedulerAwaitable{this, task, spawnJob};
}

ps_framework::Scheduler::schedulerAwaitable ps_framework::MultiTScheduler::spawnDependentIntermediate(
        coroTaskVoid *task) {
    auto spawnJob = new coroTaskVoid(
            [](auto task, auto scheduler) -> coroTaskVoid {
                scheduler->spawnIndependentIntermediate(task);
                co_return;
            }(task, this)
    );
    return schedulerAwaitable{this, task, spawnJob};
}


void ps_framework::MultiTScheduler::spawnHandler(std::coroutine_handle<promise_type_void> *handler) {
    // Lock or wait until unlocked
    std::unique_lock lock(handlerQueueMutex);
    cvHandler.wait(lock, [this]{return true;});

    handleTaskAddrs.push(handler->address());
    // Unlock
    lock.unlock();
    cvHandler.notify_one();
}





// For wrapping a task into another coroutine that
// will handle destruction and dependency
ps_framework::coroTaskVoid ps_framework::MultiTScheduler::runTask(
        ps_framework::coroTaskVoid *task,
        std::map<int ,coroTaskVoid*> *map,
        std::mutex *mapMutex,
        std::condition_variable *mapCV) {

    while (true) {
        // Resume task
        task->resume();
        // Check if transferred
        if (task->handle_.promise().transferred) {
            delete task;
            co_return;
        }
        // Lock and wait
        std::unique_lock lock(idCounterMutex);
        cv_idCounter.wait(lock, [this]{return true;});
        // Retrieve
        auto idCount = idCounter[task->handle_.promise().id];
        // Unlock
        lock.unlock();
        cv_idCounter.notify_one();

        if (task->done() || idCount > 0) {
            break;
        }
    }
    if (!task->done()){
        // Lock and wait
        std::unique_lock lock(idCounterMutex);
        cv_idCounter.wait(lock, [this]{return true;});
        // Retrieve
        auto idCount = idCounter[task->handle_.promise().id];
        // Unlock
        lock.unlock();
        cv_idCounter.notify_one();

        if (idCount > 0){
            std::unique_lock lock((*mapMutex));
            (*mapCV).wait(lock, [this]{return true;});
            (*map)[
                    task->handle_.promise().id
            ]
                    = task;
            lock.unlock();
            (*mapCV).notify_one();
        }
    }
    else{
        // retrieve parent id and deduct
        auto parentId = task->handle_.promise().parentId;
        if (parentId != -1) {
            std::unique_lock lock(idCounterMutex);
            cv_idCounter.wait(lock, [this]{return true;});
            // Retrieve and decrement
            auto idCount = --idCounter[parentId];
            // Parent task has no dependencies
            // So it is pushed to active queue
            if(idCount == 0) {
                // Unlock
                lock.unlock();
                cv_idCounter.notify_one();
                // Lock or wait until unlocked
                std::unique_lock lock((*mapMutex));
                (*mapCV).wait(lock, [this]{return true;});

                coroTaskVoid* parentTask = (*map)[parentId];
                map->erase(parentId);

                // Unlock
                lock.unlock();
                (*mapCV).notify_one();

                auto job = new coroTaskVoid(runTask(parentTask, map, mapMutex, mapCV));
                threadPool->AddJob(job);
            }
            else {
                // Unlock
                lock.unlock();
                cv_idCounter.notify_one();
            }
        }
        // The task is done so we will destroy it
        task->destroyHandler();
        delete task;
    }
    co_return;
}


void ps_framework::MultiTScheduler::runHandlers() {
    // Lock or wait until unlocked
    std::unique_lock lock(handlerQueueMutex);
    cvHandler.wait(lock, [this]{return true;});
    while(!handleTaskAddrs.empty()) {
        auto addr = handleTaskAddrs.front();
        handleTaskAddrs.pop();
        auto handler = std::coroutine_handle<ps_framework::promise_type_void>::from_address(addr);
        handler.promise().transferred = false;
        auto task = new coroTaskVoid(handler);
        auto job = new coroTaskVoid(runTask(task, &pendingTasks, &pendingQueueMutex, &cvPending));
        threadPool->AddJob(job);
    }
    // Unlock
    lock.unlock();
    cvHandler.notify_one();
}



void ps_framework::MultiTScheduler::run() {
    // Wait until all tasks have finished
    threadPool->waitUntilFinished();
    while (!pendingTasks.empty()) {
        // Run the first activeIntermediateTask
        while (!activeIntermediateTasks.empty()) {
            while (!activeIntermediateTasks.empty()) {
                auto task = activeIntermediateTasks.front();
                activeIntermediateTasks.pop();
                auto job = new coroTaskVoid(
                        runTask(task, &pendingIntermediateTasks, &pendingInterQueueMutex, &cvPendingInter));
                threadPool->AddJob(job);
                threadPool->waitUntilFinished();
            }
            threadPool->waitUntilFinished();
        }
        threadPool->waitUntilFinished();
        // Run handlers
        runHandlers();
        threadPool->waitUntilFinished();
    }
}


#endif //MINIMUMMEANCYCLE_MULTITSCHEDULER_HPP
