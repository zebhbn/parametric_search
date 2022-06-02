#include "ps_typedefs.hpp"
#include "co_task.hpp"
#include "Scheduler.hpp"
#include "PSCore.hpp"
#include <coroutine>
#include <vector>
#include <tuple>
#include <map>
#include <atomic>
#include <cmath>



//using idType = std::atomic_uint64_t;
using idType = int;

namespace ps_framework {
    template <typename T>
    class ComparisonResolver{
    public:
        ComparisonResolver(Scheduler *s1, PSCore *p1, IComparer<T> *ic1);
        co_task_void resolveComparisons();
        auto compare(T t1, T t2) {
            auto id = setCmpAndGetId(t1, t2);
            return comparerAwaitable{id, this};
        };
        struct comparerAwaitable {
            idType cmpId;
            ComparisonResolver<T> *comparisonResolver;
//            T t1;
//            T t2;
            bool await_ready() { return false; }
            void await_suspend(std::coroutine_handle<promise_type_void> h) {
                // Check if comparison can be resolved directly without computing critical value
                // Else ˇ
//                cmpId = comparisonResolver->setCmpAndGetId(t1, t2);
                // Spawn handler in scheduler
                // (these should only run when resolveComparison has run one iteration)
                comparisonResolver->scheduler.spawnHandler(h);
                // Also spawn critical value computation coroutine
                // (this object should be dependent on these) use resolveComparisonId
//                auto task = comparisonResolver->computeCriticalValue(t1, t2, cmpId);
//                comparisonResolver->scheduler.spawn(task, (*(comparisonResolver->resolveComparisonId)));
            }
            cmp_res await_resume() {
                auto tmp = comparisonResolver->getRes(cmpId);
                comparisonResolver->clearUpCmp(cmpId);
                return tmp;
            }
        };
    private:
        auto getNewId();
        idType setCmpAndGetId(T t1, T t2);
        cmp_res getRes(idType id);
        void setRes(idType id, cmp_res);
        void clearUpCmp(idType id);
//        int* resolveComparisonId;
        co_task_void computeCriticalValue(T t1,T t2, int id);
        std::map<idType, double> criticalValueMap;
        std::map<idType, std::tuple<T,T>> comparisonMap;
        std::map<idType, cmp_res> resMap;
        Scheduler *scheduler;
        PSCore *psCore;
        IComparer<T> *iComparer;
        idType idCounter;
        // Should be removed shortly
        void generateCritVec(std::vector<criticalValueResult> *critVec);
    };
}

template <typename T>
auto ps_framework::ComparisonResolver<T>::getNewId() {
    return ++idCounter;;
}


template <typename T>
ps_framework::ComparisonResolver<T>::ComparisonResolver(ps_framework::Scheduler *s1, ps_framework::PSCore *p1, ps_framework::IComparer<T> *ic1){
    scheduler = s1;
    psCore = p1;
    iComparer = ic1;
};

template <typename T>
void ps_framework::ComparisonResolver<T>::clearUpCmp(idType id) {
    // Clear data associated with id
    criticalValueMap.erase(id);
    comparisonMap.erase(id);
    resMap.erase(id);
}



template <typename T>
ps_framework::co_task_void ps_framework::ComparisonResolver<T>::computeCriticalValue(T t1, T t2, int id) {
    // Compute critical value
    double cv = iComparer->getCriticalValue(t1, t2);
    if (std::isnan(cv)) {
        setRes(id, iComparer->getCompareResult(t1,t2,Unresolved));
    }
    else {
        criticalValueMap[id] = cv;
    }
    // Set critical value
//    criticalValueVec[id] = 1.0;
}

template <typename T>
ps_framework::cmp_res ps_framework::ComparisonResolver<T>::getRes(idType id) {
    return resMap[id];
}

template <typename T>
void ps_framework::ComparisonResolver<T>::setRes(idType id, ps_framework::cmp_res res) {
    resMap[id] = res;
}


template <typename T>
idType ps_framework::ComparisonResolver<T>::setCmpAndGetId(T t1, T t2) {
    auto id = getNewId();
    comparisonMap[id] = std::make_tuple(t1, t2);
    return id;
}

// Generates criticalValueResult vector (should be removed when psCore has been rewritten)
template <typename T>
void ps_framework::ComparisonResolver<T>::generateCritVec(std::vector<criticalValueResult> *critVec) {
    for (auto const& [id, cVal] : criticalValueMap) {
        // index
        struct criticalValueResult cvr = {
                cVal,
                Unresolved,
                id
        };
        // Push data to vector
        critVec->push_back(cvr);
    }
}

template <typename T>
ps_framework::co_task_void ps_framework::ComparisonResolver<T>::resolveComparisons() {
    // Generate coroutines for computation of critical values
    for (auto const& [id, val] : comparisonMap) {
//        std::cout << key        // string (key)
//                  << ':'
//                  << val        // string's value
//                  << std::endl;
        auto task = computeCriticalValue(val[0], val[1], id);
        scheduler->spawnDependentIntermediate(task);
    }
    //  co_suspend
    co_await std::suspend_always();
    // Generate critical value result vector
    // Create vector to store critical values
    std::vector<criticalValueResult> seqResVec;
//    seqResVec.reserve(.size());

    generateCritVec(&seqResVec);
    // Call ps core on the list of critical values (those have been computed already)
    psCore->runCompareList(&seqResVec);
    // Use the result list of PSCore to compute actual comparison results
    for (auto cmpRes : (seqResVec)) {
        auto t1 = comparisonMap[cmpRes.index][0];
        auto t2 = comparisonMap[cmpRes.index][1];
        auto res = iComparer->getCompareResult(t1,t2,cmpRes.compareResult);
        setRes(cmpRes.index, res);
    }
    // Set the actual comparison results into vector
}





