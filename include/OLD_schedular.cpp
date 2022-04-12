//
// Created by zeb on 05/04/2022.
//

#include "schedular.hpp"
#include "LinearFunction.hpp"
#include <cmath>


template <typename T>
ps_framework::Schedular<T>::Schedular(IPSCore *core, IComparer<T> *cmpr) :
        psCore(core), comparer(cmpr), activeTasks(), comparisonList(), idleTasks() {}

template <typename T>
void ps_framework::Schedular<T>::runActiveTasks() {
    while (!activeTasks.empty()){
        // Run task
        activeTasks.front()->resume();
        // Push to idle tasks
        idleTasks.push(activeTasks.front());
        // Remove from queue
        activeTasks.pop();
    }
}

template <typename T>
void ps_framework::Schedular<T>::runIdleTasks() {
    while (!idleTasks.empty()){
        // Run task
        idleTasks.front()->resume();
        // If tasks is not complete then push to active tasks
        if (!idleTasks.front()->done())
            idleTasks.push(idleTasks.front());
        // Remove from queue
        idleTasks.pop();
    }
}

template <typename T>
void ps_framework::Schedular<T>::run() {
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
void ps_framework::Schedular<T>::computeCriticalValues(std::vector<criticalValueResult> *critVec) {
    for (int i = 0; i < comparisonList.size(); i++){
        // Compute critical value
        double criticalValue = comparer->getCriticalValue(
                comparisonList[i].elm1,
                comparisonList[i].elm2
                );
        if (std::isnan(criticalValue))
            return;
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
void ps_framework::Schedular<T>::resolveComparisons() {
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
void ps_framework::Schedular<T>::spawn(ps_framework::co_task_void *task) {
    // Add the co_task to active tasks queue
    activeTasks.push(task);
}

template <typename T>
void ps_framework::Schedular<T>::addComparison(T *elm1, T *elm2, cmp_res *cmpRes) {
    struct comparisonData cmpData = {elm1, elm2, cmpRes};
    comparisonList.push_back(cmpData);
}

template class ps_framework::Schedular<ps_framework::LinearFunction>;

