#include "co_task.hpp"
#include <queue>


namespace ps_framework {
    class Scheduler {
    public:
        Scheduler();
        void spawnIndependent(co_task_void *task);
        void spawnDependent(const co_task_void& task);
        void spawnDependent(const co_task_void& task, int id);

    private:
        std::queue<co_task_void*> activeTasks;
        std::queue<co_task_void*> pendingTasks;
        std::queue<std::coroutine_handle<promise_type_void>> handleTasks;
    };

}



void ps_framework::Scheduler::spawnIndependent(co_task_void *task) {
    activeTasks.push(task);
}