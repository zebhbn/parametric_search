//
// Created by zeb on 05/04/2022.
//

#include "ps_interfaces.hpp"
#include "co_task.hpp"
#include <queue>
#include <vector>

#ifndef TEST_SCHEDULAR_HPP
#define TEST_SCHEDULAR_HPP

namespace ps_framework {
    template <typename T>
    class Schedular : ps_framework::ISchedular {
    public:
        Schedular(IPSCore *core, IComparer<T>* cmpr);
        void spawn(ps_framework::co_task<void>*) override;
        void run();
        void addComparison(T*, T*, cmp_res*);
    private:
        ps_framework::IComparer<T> *comparer;
        ps_framework::IPSCore *psCore;
        std::queue<co_task<void>*> activeTasks;
        std::queue<co_task<void>*> idleTasks;
        void runActiveTasks();
        void runIdleTasks();
        void resolveComparisons();
        void computeCriticalValues(std::vector<criticalValueResult>*);

        // Structure for storing comparison data
        struct comparisonData {
            T* elm1;
            T* elm2;
            cmp_res* resPointer;
        };
        std::vector<comparisonData> comparisonList;
    };
}


#endif TEST_SCHEDULAR_HPP
