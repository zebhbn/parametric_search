//
// Created by zeb on 05/04/2022.
//

#include "ps_interfaces.hpp"
#include "co_task.hpp"
#include <queue>
#include <vector>
#include <cmath>
#include <cassert>
#include <functional>

#ifndef TEST_I_SCHEDULAR_HPP
#define TEST_I_SCHEDULAR_HPP

namespace ps_framework {
    template <typename T>
    void invoke(T func, cmp_res res){func(res);};

    template <typename T>
    class ImprovedScheduler : public ps_framework::ISchedular<T> {
    public:
        ImprovedScheduler(IPSCore *core, IComparer<T>* cmpr);
        ~ImprovedScheduler();
        void spawn(ps_framework::co_task_void*);
        void run();
        void addComparison(T*, T*, cmp_res*);
//        void addComparison(T*, T*, void (*callback)(cmp_res));
        void addComparison(T*, T*, std::function<void(ps_framework::cmp_res)>);
//        void bobaddComparison(T*, T*, );
        void resolveComparisons();
        void resolveCallbackComparisons();
    private:
        ps_framework::IComparer<T> *comparer;
        ps_framework::IPSCore *psCore;
        std::queue<co_task_void*> activeTasks;
        std::queue<co_task_void*> idleTasks;
        void runActiveTasks();
        void runIdleTasks();
        void computeCriticalValues(std::vector<criticalValueResult>*);

        // Structure for storing comparison data
        struct comparisonData {
            T* elm1;
            T* elm2;
            cmp_res* resPointer;
        };
        struct comparisonCallbackData {
            T* elm1;
            T* elm2;
            std::function<void (ps_framework::cmp_res)> callback;
        };
        std::vector<comparisonData> comparisonList;
        std::vector<comparisonCallbackData> comparisonCallbackList;
    };
}

template <typename T>
ps_framework::ImprovedScheduler<T>::ImprovedScheduler(IPSCore *core, IComparer<T> *cmpr) :
        psCore(core), comparer(cmpr), activeTasks(), comparisonList(), idleTasks() {}

template <typename T>
void ps_framework::ImprovedScheduler<T>::runActiveTasks() {
    while (!activeTasks.empty()){
        // Run task
//        assert(!activeTasks.front()->done());
        activeTasks.front()->resume();
        activeTasks.front()->done();
        // Push to idle tasks if not done
        if (!activeTasks.front()->done()){
            idleTasks.push(activeTasks.front());
        }
        else{
            // clean up
            activeTasks.front()->destroyMe();
        }
        // Remove from queue
        activeTasks.pop();
    }
}

template <typename T>
void ps_framework::ImprovedScheduler<T>::runIdleTasks() {
    while (!idleTasks.empty()){
//        std::cout<<"Resuming idle tasks"<<std::endl;
//        assert(!idleTasks.front()->done());
        // Run task
        idleTasks.front()->resume();
        // If tasks is not complete then push to active tasks
        // DONE: FIX CLEAN UP
//        if (!idleTasks.front()->done())
//            activeTasks.push(idleTasks.front());
//        else
            // clean up
            idleTasks.front()->destroyMe();
        // clean up
//        idleTasks.front()->destroyMe();
        // Remove from queue
        idleTasks.pop();
    }
}

template <typename T>
void ps_framework::ImprovedScheduler<T>::run() {
    // Repeat until no active tasks
    while(!activeTasks.empty()) {
        // Run Active tasks
        runActiveTasks();

        // Resolve comparisons
        resolveComparisons();

        // Run idle tasks
        runIdleTasks();
    }
}

template <typename T>
void ps_framework::ImprovedScheduler<T>::computeCriticalValues(std::vector<criticalValueResult> *critVec) {
    for (int i = 0; i < comparisonCallbackList.size(); i++){
        // Compute critical value
        double criticalValue = comparer->getCriticalValue(
                comparisonCallbackList[i].elm1,
                comparisonCallbackList[i].elm2
        );
//        if (std::isnan(criticalValue))
//            return;
        // Store critical value alongside
        // index
        struct criticalValueResult cvr = {
                criticalValue,
                Unresolved,
                i
        };
        // Push data to vector
        critVec->push_back(cvr);
    }
}

template <typename T>
void ps_framework::ImprovedScheduler<T>::resolveComparisons() {
    // Create vector to store critical values
    std::vector<criticalValueResult> seqResVec;
    seqResVec.reserve(comparisonList.size());

    // Parallel computation of critical values
    // TODO: Make parallel
    computeCriticalValues(&seqResVec);

    // Call sequential algorithm from pscore
    psCore->runCompareList(&seqResVec);



    // Write back the values
    for (auto cmpRes : (seqResVec)) {
        auto cmpData = comparisonList[cmpRes.index];
        auto res = comparer->getCompareResult(
                cmpData.elm1,
                cmpData.elm2,
                cmpRes.compareResult
        );
        *(cmpData.resPointer) = res;
    }

    // Cleanup
    // TODO: destroy structs in seqResVec
    // Clear comparison list
    comparisonList.clear();

}

template <typename T>
void ps_framework::ImprovedScheduler<T>::resolveCallbackComparisons() {
    // Create vector to store critical values
    std::vector<criticalValueResult> seqResVec;
    seqResVec.reserve(comparisonCallbackList.size());

    // Parallel computation of critical values
    // TODO: Make parallel
    computeCriticalValues(&seqResVec);

    // Call sequential algorithm from pscore
    psCore->runCompareList(&seqResVec);



    // Write back the values
    for (auto cmpRes : (seqResVec)) {
        comparisonCallbackData cmpData = comparisonCallbackList[cmpRes.index];
        auto res = comparer->getCompareResult(
                cmpData.elm1,
                cmpData.elm2,
                cmpRes.compareResult
        );
        // Call function
//        (cmpData->callback(res));
        (cmpData.callback(res));
    }

    // Cleanup
    // TODO: destroy structs in seqResVec
    // Clear comparison list
    comparisonCallbackList.clear();

}

template <typename T>
void ps_framework::ImprovedScheduler<T>::spawn(ps_framework::co_task_void *task) {
    // Add the co_task to active tasks queue
    activeTasks.push(task);
}

template <typename T>
void ps_framework::ImprovedScheduler<T>::addComparison(T *elm1, T *elm2, cmp_res *cmpRes) {
    struct comparisonData cmpData = {elm1, elm2, cmpRes};
    comparisonList.push_back(cmpData);
}

template <typename T>
void ps_framework::ImprovedScheduler<T>::addComparison(T *elm1, T *elm2, std::function<void (ps_framework::cmp_res)> callback) {
    struct comparisonCallbackData cmpData = {elm1, elm2, callback};
    comparisonCallbackList.push_back(cmpData);
}

template<typename T>
ps_framework::ImprovedScheduler<T>::~ImprovedScheduler() {
}


#endif //TEST_I_SCHEDULAR_HPP
