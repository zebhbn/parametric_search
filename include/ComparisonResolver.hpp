#include "PSTypedefs.hpp"
#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "PSCore.hpp"
#include <coroutine>
#include <vector>
#include <tuple>
#include <map>
#include <atomic>
#include <cmath>



#ifndef TEST_COMPARISONRESOLVER_HPP
#define TEST_COMPARISONRESOLVER_HPP
//using IdType = std::atomic_uint64_t;

//using IdType = int;

namespace ps_framework {
    template <typename T>
    class ComparisonResolver{
    public:
        ComparisonResolver(Scheduler *s1, IPSCore *p1, IComparer<T> *ic1);
        coroTaskVoid resolveComparisons();
        auto compare(T t1, T t2) {
            auto id = setCmpAndGetId(t1, t2);
            return comparerAwaitable{id, this};
        };
        struct comparerAwaitable {
            IdType cmpId;
            ComparisonResolver<T> *comparisonResolver;
//            T t1;
//            T t2;
            bool await_ready() { return false; }
            void await_suspend(std::coroutine_handle<promise_type_void> h) {
                // Set transferred flag to true
                // TODO: flag should also be cleared again
                h.promise().transferred = true;
                // Spawn handler in scheduler
                // (these should only run when resolveComparison has run one iteration)
//                std::cout<<"Spawning Handler"<<std::endl;
                comparisonResolver->scheduler->spawnHandler(&h);
                // Check if we need to spawn the resolveComparisons coroutine
                comparisonResolver->spawnMe();

            }
            CmpRes await_resume() {
                auto tmp = comparisonResolver->getRes(cmpId);
                comparisonResolver->clearUpCmp(cmpId);
                return tmp;
            }
        };
    protected:
        virtual IdType getNewId();
        virtual IdType setCmpAndGetId(T t1, T t2);
        virtual CmpRes getRes(IdType id);
        virtual void setRes(IdType id, CmpRes);
        virtual void clearUpCmp(IdType id);
        virtual void spawnMe();
        virtual void resetIsSpawned();
        virtual coroTaskVoid computeCriticalValue(T t1, T t2, int id);
        std::map<IdType, double> criticalValueMap;
        std::map<IdType, std::pair<T,T>> comparisonMap;
        std::map<IdType, CmpRes> resMap;
        Scheduler *scheduler;
        IPSCore *psCore;
        IComparer<T> *iComparer;
        IdType idCounter;
        // Should be removed shortly
        void generateCmpCvVec(std::vector<CmpCvResult> *cmpCvVec);
        // Used for spawning resolveComparisons
        bool isResolveComparisonsSpawned;
    };
}

template <typename T>
void ps_framework::ComparisonResolver<T>::spawnMe() {
//    std::cout<<"Spawning myself"<<std::endl;
    if (!isResolveComparisonsSpawned) {
        auto task = new coroTaskVoid(resolveComparisons());
        scheduler->spawnIndependentIntermediate(task);
        isResolveComparisonsSpawned = true;
    }
}

template <typename T>
void ps_framework::ComparisonResolver<T>::resetIsSpawned() {
    isResolveComparisonsSpawned = false;
}

template <typename T>
ps_framework::IdType ps_framework::ComparisonResolver<T>::getNewId() {
    return ++idCounter;;
}


template <typename T>
ps_framework::ComparisonResolver<T>::ComparisonResolver(ps_framework::Scheduler *s1, ps_framework::IPSCore *p1, ps_framework::IComparer<T> *ic1){
    scheduler = s1;
    psCore = p1;
    iComparer = ic1;
    resetIsSpawned();
};

template <typename T>
void ps_framework::ComparisonResolver<T>::clearUpCmp(IdType id) {
    // Clear data associated with id
    criticalValueMap.erase(id);
    comparisonMap.erase(id);
    resMap.erase(id);
}



template <typename T>
ps_framework::coroTaskVoid ps_framework::ComparisonResolver<T>::computeCriticalValue(T t1, T t2, int id) {
//    std::cout<<"Doing job"<<std::endl;
    // Compute critical value
    double cv = iComparer->getCriticalValue(t1, t2);
    if (std::isnan(cv)) {
        setRes(id, iComparer->getCompareResult(t1,t2,Unresolved));
    }
    else {
        criticalValueMap[id] = cv;
    }
    co_return;
}

template <typename T>
ps_framework::CmpRes ps_framework::ComparisonResolver<T>::getRes(IdType id) {
    return resMap[id];
}

template <typename T>
void ps_framework::ComparisonResolver<T>::setRes(IdType id, ps_framework::CmpRes res) {
    resMap[id] = res;
}


template <typename T>
ps_framework::IdType ps_framework::ComparisonResolver<T>::setCmpAndGetId(T t1, T t2) {
    auto id = getNewId();
    comparisonMap.insert({id, {t1, t2}});
    return id;
}

// Generates criticalValueResult vector (should be removed when psCore has been rewritten)
template <typename T>
void ps_framework::ComparisonResolver<T>::generateCmpCvVec(std::vector<CmpCvResult> *cmpCvVec) {
    for (auto const& [id, cVal] : criticalValueMap) {
        // index
        struct CmpCvResult cvr = {
                cVal,
                Unresolved,
                id
        };
        // Push data to vector
        cmpCvVec->push_back(cvr);
    }
}

template <typename T>
ps_framework::coroTaskVoid ps_framework::ComparisonResolver<T>::resolveComparisons() {
//    std::cout << "Resolving comparisons" << std::endl;
    // Generate coroutines for computation of critical values
    for (auto const& [id, val] : comparisonMap) {
        auto task = new coroTaskVoid(computeCriticalValue(val.first, val.second, id));
//        std::cout << "Spanwning" << std::endl;
        co_await scheduler->spawnDependentIntermediate(task);
    }
    //  Suspend if necessary
    if (comparisonMap.size() > 0) {
        co_await std::suspend_always();
    }
    // Generate critical value result vector
    // Create vector to store critical values
    std::vector<CmpCvResult> seqResVec;
    seqResVec.reserve(comparisonMap.size());

    generateCmpCvVec(&seqResVec);
    // Call ps core on the list of critical values (those have been computed already)
    psCore->runCompareList(&seqResVec);
    // Use the result list of PSCore to compute actual comparison results
    for (auto cmpRes : (seqResVec)) {
        auto t1 = (comparisonMap[cmpRes.index]).first;
        auto t2 = (comparisonMap[cmpRes.index]).second;
        auto res = iComparer->getCompareResult(t1,t2,cmpRes.compareResult);
        setRes(cmpRes.index, res);
    }
    // Reset spawn flag
    resetIsSpawned();
    // Set the actual comparison results into vector
    co_return;
}


#endif // test



