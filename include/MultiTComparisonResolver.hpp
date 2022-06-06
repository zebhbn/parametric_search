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
        std::mutex criticalValueMutex;
        std::condition_variable criticalValueCV;
        std::atomic_flag isResolveComparisonsSpawned = ATOMIC_FLAG_INIT;
    };
}


template <typename T>
void ps_framework::MultiTComparisonResolver<T>::spawnMe() {
    if (!isResolveComparisonsSpawned.test_and_set(std::memory_order_acquire)) {
        auto task = new coroTaskVoid(resolveComparisons());
        scheduler->spawnIndependentIntermediate(task);
    }
}

template <typename T>
void ps_framework::MultiTComparisonResolver<T>::resetIsSpawned() {
    isResolveComparisonsSpawned.clear(std::memory_order_release);
}

#endif //MINIMUMMEANCYCLE_MULTITCOMPARISONRESOLVER_HPP
