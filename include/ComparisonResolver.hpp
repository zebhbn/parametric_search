#include "ps_typedefs.hpp"
#include "co_task.hpp"
#include "Scheduler.hpp"
#include "PSCore.hpp"
#include <coroutine>
#include <vector>



namespace ps_framework {
    template <typename T>
    class ComparisonResolver{
    public:
        ComparisonResolver(const Scheduler &s1, const PSCore &p1);
        co_task_void resolveComparisons();
        auto compare(T t1, T t2) {
            return comparer_awaitable{-1, this, t1, t2};
        };
        struct comparer_awaitable {
            int cmpId;
            ComparisonResolver<T> comparisonResolver;
            T t1;
            T t2;
            bool await_ready() { return false; }
            void await_suspend(std::coroutine_handle<> h) {
                // Check if comparison can be resolved directly without computing critical value
                // Else ˇ
                cmpId = comparisonResolver.setCmpAndGetId(t1, t2);
                // Spawn handler in scheduler
                // (these should only run when resolveComparison has run one iteration)
                // Also spawn critical value computation coroutine
                // (this object should be dependent on these) use resolveComparisonId
                auto task = comparisonResolver.computeCriticalValue(t1, t2, cmpId);
                // spawn(task, (*resolveComparisonId));

            }
            cmp_res await_resume() {return comparisonResolver.getRes(cmpId);}
        };
    private:
        int setCmpAndGetId(T t1, T t2);
        cmp_res getRes(int id);
        int* resolveComparisonId;
        co_task_void computeCriticalValue(T t1,T t2, int id);
        std::vector<double> criticalValueVec;
        Scheduler &scheduler;
        PSCore &psCore;
    };
}

template <typename T>
ps_framework::ComparisonResolver<T>::ComparisonResolver(const ps_framework::Scheduler &s1, const ps_framework::PSCore &p1){
    scheduler = s1;
    psCore = p1;
};


template <typename T>
ps_framework::co_task_void ps_framework::ComparisonResolver<T>::computeCriticalValue(T t1, T t2, int id) {
    // Set critical value
    criticalValueVec[id] = 1.0;
}


template <typename T>
int ps_framework::ComparisonResolver<T>::setCmpAndGetId(T t1, T t2) {

}

template <typename T>
ps_framework::co_task_void ps_framework::ComparisonResolver<T>::resolveComparisons() {
    // Call ps core on the list of critical values (those have been computed already)
    // Use the result list of PSCore to compute actual comparison results
    // Set the actual comparison results into vector
}





