#include "co_task.hpp"
#include <queue>
#include <map>
#include <atomic>
#include <cassert>



#ifndef TEST_SCHEDULER_HPP
#define TEST_SCHEDULER_HPP

namespace ps_framework {
    class Scheduler {
    public:
        void run();
        void spawnIndependent(coro_task_void *task);
        void spawnIndependentIntermediate(coro_task_void *task);
        auto spawnDependent(coro_task_void *task);
        auto spawnDependentIntermediate(coro_task_void *task);
//        void spawnHandler(std::coroutine_handle<promise_type_void> *handler);
        void spawnHandler(void* coroPointer);

        // Awaitable to enable co_await support on spawn command
        struct schedulerAwaitable {
            Scheduler* scheduler;
            coro_task_void *dependentTask;
            bool await_ready() { return false; }
            bool await_suspend(std::coroutine_handle<promise_type_void> h) {
                // Increment id dependent counter
                scheduler->idCounter[h.promise().id]++;
                std::cout << "Setting task: "
                          << dependentTask->handle_.promise().parentId
                          << "parendId to: "
                          << h.promise().id
                          << std::endl;
                dependentTask->handle_.promise().parentId = h.promise().id;
                // Resume the current coroutine
                return false;
            }
            void await_resume() {return;}
        };

    private:
        std::map<int, int> idCounter;
        std::queue<coro_task_void*> activeTasks;
        std::map<int, coro_task_void*> pendingTasks;
        std::queue<coro_task_void*> activeIntermediateTasks;
        std::map<int, coro_task_void*> pendingIntermediateTasks;
//        std::queue<std::coroutine_handle<promise_type_void>*> handleTasks;
        std::queue<void*> handleTaskAddrs;
        std::atomic_int atomicId;
        int getNewId();
        int initNewIdCount();
        void runActiveTasks();
        void runActiveIntermediateTasks();
        void runHandlers();
    };

}

int ps_framework::Scheduler::getNewId() {
    return ++atomicId;
}

int ps_framework::Scheduler::initNewIdCount() {
    auto id = getNewId();
    idCounter[id] = 0;
    return id;
}



void ps_framework::Scheduler::spawnIndependent(coro_task_void *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeTasks.push(task);
}

void ps_framework::Scheduler::spawnIndependentIntermediate(coro_task_void *task){
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeIntermediateTasks.push(task);
}

auto ps_framework::Scheduler::spawnDependentIntermediate(coro_task_void *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeIntermediateTasks.push(task);
    // Return awaitable
    return schedulerAwaitable{this, task};
};

auto ps_framework::Scheduler::spawnDependent(coro_task_void *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeTasks.push(task);
    //
    std::cout<<"Spawned dependent task with new id: "<<newId<<std::endl;
    // Return awaitable
    return schedulerAwaitable{this, task};
};

//void ps_framework::Scheduler::spawnHandler(std::coroutine_handle<promise_type_void> *handler) {
void ps_framework::Scheduler::spawnHandler(void *coroPointer) {
    // Push handler to handle queue
    handleTaskAddrs.push(coroPointer);
}

void ps_framework::Scheduler::runActiveTasks() {
    while (!activeTasks.empty()){
        // Print queue status
        std::cout<<"Active queue contents"<<std::endl;
        auto tmp = activeTasks;
        while (!tmp.empty()) {
            std::cout<<"    ID: "<<tmp.front()->handle_.promise().id<<std::endl;
            tmp.pop();
        }
        // Run task
        std::cout<<"Running task with id: "<<activeTasks.front()->handle_.promise().id<<std::endl;
        std::cout<<"Running task with parent id: "<<activeTasks.front()->handle_.promise().parentId<<std::endl;
        activeTasks.front()->resume();
        std::cout<<"Control returned from task with id: "<<activeTasks.front()->handle_.promise().id<<std::endl;
        std::cout<<"Status of task done: "<< activeTasks.front()->done() << std::endl;
//        activeTasks.front()->done();
        if (activeTasks.front()->handle_.promise().transferred) {
            activeTasks.pop();
            continue;
        }
        // If task is not done then assume that
        // it has other task which it depends on to be finished
        if (!activeTasks.front()->done()){
            if (idCounter[activeTasks.front()->handle_.promise().id] > 0){
                std::cout<<"Pushing task: "<<activeTasks.front()->handle_.promise().id<<" to pendingTasks"<<std::endl;
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
            activeTasks.front()->destroyMe();
        }
        // Remove from queue
        activeTasks.pop();
    }
}


void ps_framework::Scheduler::runActiveIntermediateTasks() {
    while (!activeIntermediateTasks.empty()){
        // Run task
        std::cout<<"Active queue contents"<<std::endl;
        auto tmp = activeIntermediateTasks;
        while (!tmp.empty()) {
            std::cout<<"    ID: "<<tmp.front()->handle_.promise().id<<std::endl;
            tmp.pop();
        }
        activeIntermediateTasks.front()->resume();
//        activeTasks.front()->done();
        // If task is not done then assume that
        // it has other task which it depends on to be finished
        if (!activeIntermediateTasks.front()->done()){
            std::cout<<idCounter[activeIntermediateTasks.front()->handle_.promise().id]<<std::endl;
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
            activeIntermediateTasks.front()->destroyMe();
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

//        std::cout<<"Running handlers, size of queue: "<<handleTasks.size()<<std::endl;
//        std::cout<<"Running handler with ptr: "<<&(handleTasks.front())<< std::endl;
//        std::cout<<"Running handler with id: "<<handleTasks.front()->promise().id<< std::endl;
//        handleTasks.front()->resume();
        handler.resume();
//        assert(handleTasks.front()->done());
        // retrieve parent id and deduct
//        auto parentId = handleTasks.front()->promise().parentId;
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
//        handleTasks.front()->destroy();
//        handleTasks.pop();
        handler.destroy();
        handleTaskAddrs.pop();
    }
}



void ps_framework::Scheduler::run(){
//    while ((!activeTasks.empty()) || (!activeIntermediateTasks.empty()) || (!handleTasks.empty()) ) {
    while ((!activeTasks.empty()) || (!activeIntermediateTasks.empty()) || (!handleTaskAddrs.empty()) ) {
        // Run active tasks
        runActiveTasks();
        std::cout<<"Finished active tasks"<<std::endl;
        // Run intermediate tasks
        runActiveIntermediateTasks();
        std::cout<<"Finished intermediate tasks"<<std::endl;
        // Run handlers
        runHandlers();
        std::cout<<"Finished handler tasks"<<std::endl;

        std::cout<<"Pending queue size: "<<pendingTasks.size()<<std::endl;
        std::cout<<"Pending inter queue size: "<<pendingIntermediateTasks.size()<<std::endl;
    }
}

#endif // TEST