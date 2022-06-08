//
// Created by zeb on 06/06/2022.
//

#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <mutex>
#include <condition_variable>

#ifndef MINIMUMMEANCYCLE_MULTITCOMPARISONRESOLVER_HPP
#define MINIMUMMEANCYCLE_MULTITCOMPARISONRESOLVER_HPP

namespace ps_framework{
    template <typename T>
    class MultiTComparisonResolver : public ComparisonResolver<T> {
    public:
        MultiTComparisonResolver(Scheduler *s, IPSCore *core, IComparer<T> *c) : ComparisonResolver<T>(s,core,c) {};
    protected:
        std::mutex resMapMutex;
        std::condition_variable resMapCV;
        std::mutex critMapMutex;
        std::condition_variable critMapCV;
        std::mutex comparisonMapMutex;
        std::condition_variable comparisonMapCV;
        std::mutex idCounterMutex;
        std::condition_variable idCounterCV;
        std::atomic_flag isResolveComparisonsSpawned = ATOMIC_FLAG_INIT;
        IdType getNewId();
        void spawnMe();
        void resetIsSpawned();
        coroTaskVoid computeCriticalValue(T t1, T t2, int id);
        CmpRes getRes(IdType id);
        void setRes(IdType id, CmpRes res);
        void clearUpCmp(IdType id);
        IdType setCmpAndGetId(T t1, T t2);
//        void generateCmpCvVec(std::vector<CmpCvResult> *cmpCvVec);
        using ComparisonResolver<T>::iComparer;
        using ComparisonResolver<T>::criticalValueMap;
        using ComparisonResolver<T>::resMap;
        using ComparisonResolver<T>::comparisonMap;
        using ComparisonResolver<T>::getNewId;
        using ComparisonResolver<T>::idCounter;
    };
}

template <typename T>
ps_framework::IdType ps_framework::MultiTComparisonResolver<T>::getNewId() {
    // Increment id dependent counter
    std::unique_lock lock(idCounterMutex);
    idCounterCV.wait(lock, [this]{return true;});
    // Set value
    auto retval = ++idCounter;
    // Unlock
    lock.unlock();
    idCounterCV.notify_one();
    return retval;
}

template <typename T>
void ps_framework::MultiTComparisonResolver<T>::clearUpCmp(IdType id) {
    // Clear data associated with id
    {
        std::unique_lock lock(critMapMutex);
        critMapCV.wait(lock, [this]{return true;});
        criticalValueMap.erase(id);
    }
    critMapCV.notify_one();

    {
        std::unique_lock lock(comparisonMapMutex);
        comparisonMapCV.wait(lock, [this]{return true;});
        comparisonMap.erase(id);
    }
    comparisonMapCV.notify_one();

    {
        std::unique_lock lock(resMapMutex);
        resMapCV.wait(lock, [this]{return true;});
        resMap.erase(id);
    }
    resMapCV.notify_one();
}


template <typename T>
void ps_framework::MultiTComparisonResolver<T>::spawnMe() {
    if (!isResolveComparisonsSpawned.test_and_set(std::memory_order_acquire)) {
        auto task = new coroTaskVoid(this->resolveComparisons());
        this->scheduler->spawnIndependentIntermediate(task);
    }
}

template <typename T>
void ps_framework::MultiTComparisonResolver<T>::resetIsSpawned() {
    isResolveComparisonsSpawned.clear(std::memory_order_release);
}

template <typename T>
ps_framework::coroTaskVoid ps_framework::MultiTComparisonResolver<T>::computeCriticalValue(T t1, T t2, int id) {
    // Compute critical value
    double cv = iComparer->getCriticalValue(t1, t2);
    if (std::isnan(cv)) {
        setRes(id, iComparer->getCompareResult(t1,t2,Unresolved));
    }
    else {
        // Increment id dependent counter
        std::unique_lock lock(critMapMutex);
        critMapCV.wait(lock, [this]{return true;});
        // Set value
        criticalValueMap[id] = cv;
        // Unlock
        lock.unlock();
        critMapCV.notify_one();
    }
    co_return;
}

template <typename T>
ps_framework::CmpRes ps_framework::MultiTComparisonResolver<T>::getRes(IdType id) {
    // Increment id dependent counter
    std::unique_lock lock(resMapMutex);
    resMapCV.wait(lock, [this]{return true;});
    // Set value
    auto res = resMap[id];
    // Unlock
    lock.unlock();
    resMapCV.notify_one();
    return res;
}

template <typename T>
void ps_framework::MultiTComparisonResolver<T>::setRes(IdType id, ps_framework::CmpRes res) {
    // Increment id dependent counter
    std::unique_lock lock(resMapMutex);
    resMapCV.wait(lock, [this]{return true;});
    // Set value
    resMap[id] = res;
    // Unlock
    lock.unlock();
    resMapCV.notify_one();
}


template <typename T>
ps_framework::IdType ps_framework::MultiTComparisonResolver<T>::setCmpAndGetId(T t1, T t2) {
    auto id = getNewId();
    // Increment id dependent counter
    std::unique_lock lock(comparisonMapMutex);
    comparisonMapCV.wait(lock, [this]{return true;});
    // Insert value
    comparisonMap.insert({id, {t1, t2}});
    // Unlock
    lock.unlock();
    comparisonMapCV.notify_one();
    return id;
}

// Generates criticalValueResult vector (should be removed when psCore has been rewritten)
//template <typename T>
//void ps_framework::MultiTComparisonResolver<T>::generateCmpCvVec(std::vector<CmpCvResult> *cmpCvVec) {
//    for (auto const& [id, cVal] : criticalValueMap) {
//        // index
//        struct CmpCvResult cvr = {
//                cVal,
//                Unresolved,
//                id
//        };
//        // Push data to vector
//        cmpCvVec->push_back(cvr);
//    }
//}

#endif //MINIMUMMEANCYCLE_MULTITCOMPARISONRESOLVER_HPP
