#include "ps_interfaces.hpp"
#include "co_task.hpp"
#ifndef PS_FRAMEWORK
#define PS_FRAMEWORK


namespace ps_framework {
// The parametric framework class containing all relavant methods and fields
// This should maybe be in a namespace of its own
    template<typename T>
    class PSFramework {
    public:
        PSFramework(
                IParAlgo *,
                IComparer<T> *,
                ISeqAlgo *,
                ISchedular *,
                IPSCore *
        );

        PSFramework(
                IParAlgo *,
                IComparer<T> *,
                ISeqAlgo *,
                ISchedular *,
                IPSCore *,
                double,
                double

        );

        // For testing purposes
        PSFramework();

        void spawn(co_task<void>*);

//        void resolveComparisons();

        void compare(T *, T *, cmp_res *);

    private:
        IParAlgo *parAlgo;
        IComparer<T> *comparer;
        ISeqAlgo *seqAlgo;
        ISchedular *schedular;
        IPSCore *psCore;

        // The start of the interval where we know lambda* is in, (e)
        double start;
        // The end of the interval where we know lambda* is in, (f)
        double end;

        // Sequiential algorithm used when comparing functions
        // ISeqAlgo *seqAlgo;
        // internal function to check if value is inside the search interval
        bool isInInterval(double x);

    };

}

#endif