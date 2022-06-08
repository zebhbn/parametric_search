#include "CoTask.hpp"
#include <queue>
#include <map>
#include <atomic>
#include <cassert>
#include "PSInterfaces.hpp"



#ifndef TEST_SCHEDULER_HPP
#define TEST_SCHEDULER_HPP

namespace ps_framework {
    class Scheduler{
    public:
        // Awaitable to enable co_await support on spawn command
        struct schedulerAwaitable{
            Scheduler* scheduler;
            coroTaskVoid *dependentTask;
            coroTaskVoid *spawnJob;
            bool await_ready() { return false; }
            bool await_suspend(std::coroutine_handle<promise_type_void> h) {
                // Increment id dependent counter
                scheduler->incrementId(h.promise().id);
                dependentTask->handle_.promise().parentId = h.promise().id;
                // Run spawning task (so that we first spawn after everything has been set)
//                std::cout<<"checking if spawn job is non null"<<std::endl;
                if (spawnJob) {
                    spawnJob->resume();
                    spawnJob->destroyHandler();
                    delete spawnJob;
                }
                // Resume the current coroutine
                return false;
            }
            void await_resume() {return;}
        };
        virtual void run();
        virtual void spawnIndependent(coroTaskVoid *task);
        virtual void spawnIndependentIntermediate(coroTaskVoid *task);
        virtual schedulerAwaitable spawnDependent(coroTaskVoid *task);
        virtual schedulerAwaitable spawnDependentIntermediate(coroTaskVoid *task);
        virtual void spawnHandler(std::coroutine_handle<promise_type_void> *handler);
//        void spawnHandler(void* coroPointer);


    protected:
        std::map<int, int> idCounter;
        std::queue<coroTaskVoid*> activeTasks;
        std::map<int, coroTaskVoid*> pendingTasks;
        std::queue<coroTaskVoid*> activeIntermediateTasks;
        std::map<int, coroTaskVoid*> pendingIntermediateTasks;
//        std::queue<std::coroutine_handle<promise_type_void>*> handleTasks;
        std::queue<void*> handleTaskAddrs;
        std::atomic_int atomicId;
        int getNewId();
        int initNewIdCount();
        void runActiveTasks();
        void runActiveIntermediateTasks();
        void runHandlers();
        void incrementId(int id);
    };

}

void ps_framework::Scheduler::incrementId(int id) {
    idCounter[id]++;
}


int ps_framework::Scheduler::getNewId() {
    return ++atomicId;
}

int ps_framework::Scheduler::initNewIdCount() {
    auto id = getNewId();
    idCounter[id] = 0;
    return id;
}



void ps_framework::Scheduler::spawnIndependent(coroTaskVoid *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeTasks.push(task);
}

void ps_framework::Scheduler::spawnIndependentIntermediate(coroTaskVoid *task){
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeIntermediateTasks.push(task);
}

ps_framework::Scheduler::schedulerAwaitable ps_framework::Scheduler::spawnDependentIntermediate(coroTaskVoid *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeIntermediateTasks.push(task);
    // Return awaitable
    return schedulerAwaitable{this, task, nullptr};
};

ps_framework::Scheduler::schedulerAwaitable ps_framework::Scheduler::spawnDependent(coroTaskVoid *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeTasks.push(task);
    // Return awaitable
    return schedulerAwaitable{this, task, nullptr};
};

void ps_framework::Scheduler::spawnHandler(std::coroutine_handle<promise_type_void> *handler) {
//void ps_framework::Scheduler::spawnHandler(void *coroPointer) {
    // Push handler to handle queue
    handleTaskAddrs.push(handler->address());
}


void ps_framework::Scheduler::runActiveTasks() {
    while (!activeTasks.empty()){
        // Print queue status
        auto tmp = activeTasks;
//        while (!tmp.empty()) {
//            std::cout<<"    ID: "<<tmp.front()->handle_.promise().id<<std::endl;
//            tmp.pop();
//        }
        // Run task
        activeTasks.front()->resume();
        if (activeTasks.front()->handle_.promise().transferred) {
            delete activeTasks.front();
            activeTasks.pop();
            continue;
        }
        // If task is not done then assume that
        // it has other task which it depends on to be finished
        if (!activeTasks.front()->done()){
            if (idCounter[activeTasks.front()->handle_.promise().id] > 0){
                pendingTasks[
                        activeTasks.front()->handle_.promise().id
                ]
                        = activeTasks.front();
            }
            else {
                activeTasks.push(activeTasks.front());
            }
        }
        else{
            // retrieve parent id and deduct
            auto parentId = activeTasks.front()->handle_.promise().parentId;
            if (parentId != -1) {
                --idCounter[parentId];
                // Parent task has no dependencies
                // So it is pushed to active queue
                if(idCounter[parentId] == 0) {
                    activeTasks.push(pendingTasks[parentId]);
                    pendingTasks.erase(parentId);
                }
            }
            // The task is done so we will destroy it
//            std::cout << "Destroying handler" << std::endl;
            activeTasks.front()->destroyHandler();
//            std::cout << "Deleting" << std::endl;
//            delete activeTasks.front();
//            std::cout << "Deleted" << std::endl;
        }
        // Remove from queue
        activeTasks.pop();
    }
}


void ps_framework::Scheduler::runActiveIntermediateTasks() {
    while (!activeIntermediateTasks.empty()){
        // Run task
        activeIntermediateTasks.front()->resume();
//        activeTasks.front()->done();
        // If task is not done then assume that
        // it has other task which it depends on to be finished
        if (!activeIntermediateTasks.front()->done()){
            if(idCounter[activeIntermediateTasks.front()->handle_.promise().id] > 0) {
                pendingIntermediateTasks[
                        activeIntermediateTasks.front()->handle_.promise().id
                ]
                        = activeIntermediateTasks.front();
            }
            else {
                activeIntermediateTasks.push(activeIntermediateTasks.front());
            }
        }
        else{
            // retrieve parent id and deduct
            auto parentId = activeIntermediateTasks.front()->handle_.promise().parentId;
            if (parentId != -1) {
                --idCounter[parentId];
                // Parent task has no dependencies
                // So it is pushed to active queue
                if(idCounter[parentId] == 0) {
                    activeIntermediateTasks.push(pendingIntermediateTasks[parentId]);
                    pendingIntermediateTasks.erase(parentId);
                }
            }
            // The task is done so we will destroy it
            activeIntermediateTasks.front()->destroyHandler();
            delete activeIntermediateTasks.front();
        }
        // Remove from queue
        activeIntermediateTasks.pop();
    }
}


void ps_framework::Scheduler::runHandlers() {
//    std::cout<<"Running handlers, size of queue: "<<handleTasks.size()<<std::endl;
    while (!handleTaskAddrs.empty()) {
        // Run task
        auto handler = std::coroutine_handle<promise_type_void>::from_address(handleTaskAddrs.front());
//        std::cout << "Running handler with ID: " << handler.promise().id << std::endl;

        handler.resume();
        auto parentId = handler.promise().parentId;
        if (parentId != -1) {
            --idCounter[parentId];
            // Parent task has no dependencies
            // So it is pushed to active queue
            if(idCounter[parentId] == 0) {
                // Assume handler parent tasks are non-intermediate tasks
                activeTasks.push(pendingTasks[parentId]);
                pendingTasks.erase(parentId);
            }
        }
        // The task is done so we will destroy and pop it
        handler.destroy();
        handleTaskAddrs.pop();
    }
}



void ps_framework::Scheduler::run(){
//    while ((!activeTasks.empty()) || (!activeIntermediateTasks.empty()) || (!handleTasks.empty()) ) {
    while ((!activeTasks.empty()) || (!activeIntermediateTasks.empty()) || (!handleTaskAddrs.empty()) ) {
        // Run active tasks
        runActiveTasks();
//        std::cout<<"Finished active tasks"<<std::endl;
        // Run intermediate tasks
        runActiveIntermediateTasks();
//        std::cout<<"Finished intermediate tasks"<<std::endl;
        // Run handlers
        runHandlers();
//        std::cout<<"Finished handler tasks"<<std::endl;
//
//        std::cout<<"Pending queue size: "<<pendingTasks.size()<<std::endl;
//        std::cout<<"Pending inter queue size: "<<pendingIntermediateTasks.size()<<std::endl;
    }
}

#endif // TEST