#include "ps_typedefs.hpp"
#include "co_task.hpp"
#include <vector>

#ifndef PS_INTERFACES
#define PS_INTERFACES

namespace ps_framework {

    class IParAlgo {
    public:
        virtual void run() = 0;
    };

    class ISeqAlgo {
    public:
        virtual cmp_res compare(double lambda) = 0;
    };

    template<typename T>
    class IComparer {
    public:
        virtual double getCriticalValue(T *, T *) = 0;
        virtual double getCriticalValue(T, T) = 0;

        virtual cmp_res getCompareResult(T *, T *, cmp_res) = 0;
        virtual cmp_res getCompareResult(T, T, cmp_res) = 0;
    };

    template <typename T>
    class ISchedular {
    public:
        virtual void spawn(co_task_void*) = 0;
        virtual void addComparison(T*, T*, cmp_res*) = 0;
        virtual void run() = 0;
    };

    class IComparisonResolver {
    public:
        virtual void resolve() = 0;
    };

    class IPSCore {
    public:
        virtual void runCompareList(std::vector<criticalValueResult>*) = 0;
    };

}


#endif