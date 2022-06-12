#include "PSTypedefs.hpp"
#include "CoTask.hpp"
#include <vector>
#include <coroutine>

#ifndef PS_INTERFACES
#define PS_INTERFACES

namespace ps_framework {

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


    class IPSCore {
    public:
        virtual void runCompareList(std::vector<CmpCvResult>*) = 0;
        virtual bool isInInterval(double) = 0;
        virtual CmpRes compareToLambdaStar(double) = 0;
    };

}


#endif