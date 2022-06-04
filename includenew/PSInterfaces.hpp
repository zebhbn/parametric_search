#include "PSTypedefs.hpp"
#include "CoTask.hpp"
#include <vector>
#include <coroutine>

#ifndef PS_INTERFACES
#define PS_INTERFACES

namespace ps_framework {

    class IParAlgo {
    public:
        virtual void run() = 0;
    };

    class ISeqAlgo {
    public:
        virtual CmpRes compare(double lambda) = 0;
    };

    template<typename T>
    class IComparer {
    public:
        virtual double getCriticalValue(T *, T *) = 0;
        virtual double getCriticalValue(T, T) = 0;

        virtual CmpRes getCompareResult(T *, T *, CmpRes) = 0;
        virtual CmpRes getCompareResult(T, T, CmpRes) = 0;
    };

    class IScheduler {
    public:
        virtual void spawn(coroTaskVoid*) = 0;
//        virtual std::cor spawn() = 0;
        virtual void run() = 0;
    };

    class IComparisonResolver {
    public:
        virtual void resolve() = 0;
    };

    class IPSCore {
    public:
        virtual void runCompareList(std::vector<CmpCvResult>*) = 0;
    };

}


#endif