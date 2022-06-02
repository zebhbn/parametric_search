#include "co_task.hpp"
#include <queue>
#include <map>
#include <atomic>


namespace ps_framework {
    class Scheduler {
    public:
        Scheduler();
        void run();
        void spawnIndependent(coro_task_void *task);
        void spawnIndependentIntermediate(coro_task_void *task);
        auto spawnDependent(coro_task_void *task);
        auto spawnDependentIntermediate(coro_task_void *task);
        void spawnHandler(std::coroutine_handle<promise_type_void> handler);

        // Awaitable to enable co_await support on spawn command
        struct schedulerAwaitable {
            Scheduler* scheduler;
            bool await_ready() { return false; }
            void await_suspend(std::coroutine_handle<promise_type_void> h) {
                // Increment id dependent counter
                scheduler->idCounter[h.promise().id]++;
            }
            void await_resume() {return;}
        };

    private:
        std::map<int, int> idCounter;
        std::queue<coro_task_void*> activeTasks;
        std::queue<coro_task_void*> pendingTasks;
        std::queue<coro_task_void*> activeIntermediateTasks;
        std::queue<coro_task_void*> pendingIntermediateTasks;
        std::queue<std::coroutine_handle<promise_type_void>> handleTasks;
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
    return schedulerAwaitable{this};
};

auto ps_framework::Scheduler::spawnDependent(coro_task_void *task) {
    // Get new id and set task/promise id
    auto newId = initNewIdCount();
    task->handle_.promise().id = newId;
    // Push to active tasks
    activeTasks.push(task);
    // Return awaitable
    return schedulerAwaitable{this};
};

void ps_framework::Scheduler::spawnHandler(std::coroutine_handle<promise_type_void> handler) {
    // Push handler to handle queue
    handleTasks.push(handler);
}


void ps_framework::Scheduler::run(){
    // Run active tasks
    runActiveTasks();
    // Run intermediate tasks
    runActiveIntermediateTasks();
    // Run handlers
    runHandlers();
}

