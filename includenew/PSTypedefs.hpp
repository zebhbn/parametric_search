#include <atomic>

#ifndef PS_TYPEDEFS
#define PS_TYPEDEFS

namespace ps_framework {

    // Used for comparison results
    enum CmpRes {
        LessThan,
        EqualTo,
        GreaterThan,

        // Added to make unresolved comparisons possible
        // probably not the best solution...
        Unresolved
    };

    using CriticalValue = double;
    using AtomicId = std::atomic_uint32_t;
    using IdType = uint32_t;

    // Used to store critical values combined with
    // comparison results and indexes
    struct CmpCvResult{
        CriticalValue criticalValue;
        CmpRes compareResult;
        IdType index;
    };
}

#endif