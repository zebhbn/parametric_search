//
// Created by zeb on 05/04/2022.
//

#include "ps_interfaces.hpp"
#include "co_task.hpp"
#include <queue>
#include <vector>
#include <cmath>
#include <cassert>
#include <mutex>
#include <condition_variable>
#include "ThreadPool.hpp"

#ifndef MULTISCHEDULAR_HPP
#define MULTISCHEDULAR_HPP

namespace ps_framework {
    template <typename T>
    class MultiThreadSchedular : public ps_framework::ISchedular<T> {
    public:
        MultiThreadSchedular(IPSCore *core, IComparer<T>* cmpr);
        MultiThreadSchedular(IPSCore *core, IComparer<T>* cmpr, int numberOfThreads);
        void spawn(ps_framework::co_task_void*);
        void run();
        void addComparison(T*, T*, cmp_res*);
    private:
        ps_framework::IComparer<T> *comparer;
        ps_framework::IPSCore *psCore;
        std::queue<co_task_void*> activeTasks;
        std::queue<co_task_void*> idleTasks;
        ThreadPool* threadPool;
        void runActiveTasks();
        void runIdleTasks();
        void resolveComparisons();
        void computeCriticalValues(std::vector<criticalValueResult>*);

        // Added for thread computation of critical values
        co_task_void computeSingleCriticalValue(
                std::vector<criticalValueResult>* critVec,
                int index,
                T elm1,
                T elm2
                );

        // Added for safe storing and posible reading when computing critical values
        std::mutex critMutex;
        std::condition_variable cv;



        // Structure for storing comparison data
        struct comparisonData {
            T* elm1;
            T* elm2;
            cmp_res* resPointer;
        };
        std::vector<comparisonData> comparisonList;
    };
}

template <typename T>
ps_framework::MultiThreadSchedular<T>::MultiThreadSchedular(IPSCore *core, IComparer<T> *cmpr, int numberOfThreads) :
        psCore(core), comparer(cmpr), activeTasks(), comparisonList(), idleTasks() {
    threadPool = new ThreadPool(numberOfThreads);
}

template <typename T>
ps_framework::MultiThreadSchedular<T>::MultiThreadSchedular(IPSCore *core, IComparer<T> *cmpr) :
        psCore(core), comparer(cmpr), activeTasks(), comparisonList(), idleTasks() {
    threadPool = new ThreadPool();
}

template <typename T>
void ps_framework::MultiThreadSchedular<T>::runActiveTasks() {
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
void ps_framework::MultiThreadSchedular<T>::runIdleTasks() {
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
void ps_framework::MultiThreadSchedular<T>::run() {
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
ps_framework::co_task_void ps_framework::MultiThreadSchedular<T>::computeSingleCriticalValue(std::vector<criticalValueResult> *critVec, int index, T elm1, T elm2) {
    // Compute critical value, could take a long time
    double criticalValue = comparer->getCriticalValue(elm1, elm2);
    // Store critical value alongside
    // index
    struct criticalValueResult cvr = {
            criticalValue,
            Unresolved,
            index
    };
    // Lock or wait until unlocked
    std::unique_lock lock(critMutex);
    cv.wait(lock);

    // Push data to vector
    critVec->push_back(cvr);

    // Unlock
    lock.unlock();
    cv.notify_one();
    //Return
    co_return;
}


template <typename T>
void ps_framework::MultiThreadSchedular<T>::computeCriticalValues(std::vector<criticalValueResult> *critVec) {
    // ThreadPool should handle destruction, but solved for now by having vector of pointers
    auto tasks = new std::vector<co_task_void*>;
    tasks->reserve(critVec->size());
    for (int i = 0; i < comparisonList.size(); i++){
        auto elm1 = comparisonList[i].elm1;
        auto elm2 = comparisonList[i].elm2;
        auto job = new co_task_void(computeSingleCriticalValue(critVec, i, elm1, elm2));
        threadPool->AddJob(job);
        tasks->push_back(job);
    }
    // wait until finished
    threadPool->waitUntilFinished();
    // Again should be handled in a more elegant fashion in threadpool
    // or automatically with descriptors
    for (auto &t : *tasks){
        t->destroyMe();
    }
}

template <typename T>
void ps_framework::MultiThreadSchedular<T>::resolveComparisons() {
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
void ps_framework::MultiThreadSchedular<T>::spawn(ps_framework::co_task_void *task) {
    // Add the co_task to active tasks queue
    activeTasks.push(task);
}

template <typename T>
void ps_framework::MultiThreadSchedular<T>::addComparison(T *elm1, T *elm2, cmp_res *cmpRes) {
    struct comparisonData cmpData = {elm1, elm2, cmpRes};
    comparisonList.push_back(cmpData);
}


#endif //MULTISCHEDULAR_HPP
