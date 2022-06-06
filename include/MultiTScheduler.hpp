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
//        coroTaskVoid runIntermediateTask(coroTaskVoid*);
        coroTaskVoid runHandler(void*);
        int initNewIdCount();
        // Added for saf
        std::mutex idCounterMutex;
        std::condition_variable cv_idCounter;
        std::mutex pendingQueueMutex;
        std::condition_variable cvPending;
        std::mutex pendingInterQueueMutex;
        std::condition_variable cvPendingInter;
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
    spawnIndependent(task);
    return schedulerAwaitable{this, task};
}

ps_framework::Scheduler::schedulerAwaitable ps_framework::MultiTScheduler::spawnDependentIntermediate(
        coroTaskVoid *task) {
    spawnIndependentIntermediate(task);
    return schedulerAwaitable{this, task};
}





ps_framework::coroTaskVoid ps_framework::MultiTScheduler::runTask(
        ps_framework::coroTaskVoid *task,
        std::map<int ,coroTaskVoid*> *map,
        std::mutex *mapMutex,
        std::condition_variable *mapCV) {
    while (true) {
        task->resume();
        std::unique_lock lock(idCounterMutex);
        cv_idCounter.wait(lock, [this]{return true;});
        if (task->done() || idCounter[task->handle_.promise().id] > 0) {
            // Unlock
            lock.unlock();
            cv_idCounter.notify_one();
            break;
        }
        // Unlock
        lock.unlock();
        cv_idCounter.notify_one();
    }
    if (!task->done()){
        std::unique_lock lock(idCounterMutex);
        cv_idCounter.wait(lock, [this]{return true;});
        if (idCounter[task->handle_.promise().id] > 0){
            // Unlock
            lock.unlock();
            cv_idCounter.notify_one();

            std::unique_lock lock((*mapMutex));
            cv_idCounter.wait(lock, [this]{return true;});
            (*map)[
                    task->handle_.promise().id
            ]
                    = task;
            lock.unlock();
            (*mapCV).notify_one();
        }
        else {
            // Unlock
            lock.unlock();
            cv_idCounter.notify_one();
        }
    }
    else{
        // retrieve parent id and deduct
        auto parentId = task->handle_.promise().parentId;
        if (parentId != -1) {
            std::unique_lock lock(idCounterMutex);
            cv_idCounter.wait(lock, [this]{return true;});
            --idCounter[parentId];
            // Parent task has no dependencies
            // So it is pushed to active queue
            if(idCounter[parentId] == 0) {
                // Unlock
                lock.unlock();
                cv_idCounter.notify_one();
                // Lock or wait until unlocked
                std::unique_lock lock((*mapMutex));
                (*mapCV).wait(lock, [this]{return true;});

                auto parentTask = (*map)[parentId];
                map->erase(parentId);

                // Unlock
                lock.unlock();
                (*mapCV).notify_one();

                threadPool->AddJob(parentTask);
            }
            else {
                // Unlock
                lock.unlock();
                cv_idCounter.notify_one();
            }
        }
        // The task is done so we will destroy it
        task->destroyMe();
    }
    co_return;
}


ps_framework::coroTaskVoid ps_framework::MultiTScheduler::runHandler(void *addr) {
    auto handler = std::coroutine_handle<ps_framework::promise_type_void>::from_address(addr);
    handler.resume();
    // Destroy the handler
    handler.destroy();
    co_return;
}

void ps_framework::MultiTScheduler::runHandlers() {
    while(!handleTaskAddrs.empty()) {
        auto addr = handleTaskAddrs.front();
        handleTaskAddrs.pop();
        threadPool->AddJob(new coroTaskVoid(runHandler(addr)));
    }
}



void ps_framework::MultiTScheduler::run() {
    // Wait until all tasks have finished
    threadPool->waitUntilFinished();
    while (!pendingTasks.empty()) {
        // Run the first activeIntermediateTask
        if (!activeIntermediateTasks.empty()) {
            auto task =  activeIntermediateTasks.front();
            activeIntermediateTasks.pop();
            auto job = new coroTaskVoid(runTask(task, &pendingIntermediateTasks, &pendingInterQueueMutex, &cvPendingInter));
            threadPool->AddJob(job);
        }
        // Wait until all tasks have finished
        threadPool->waitUntilFinished();
        // Run handlers
        runHandlers();
        // Wait until all tasks have finished
        threadPool->waitUntilFinished();
    }
}


#endif //MINIMUMMEANCYCLE_MULTITSCHEDULER_HPP
