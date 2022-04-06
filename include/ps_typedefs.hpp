
#ifndef PS_TYPEDEFS
#define PS_TYPEDEFS

namespace ps_framework {

    // Used for comparison results
    enum cmp_res {
        LessThan,
        EqualTo,
        GreaterThan,

        // Added to make unresolved comparisons posible
        // probably not the best solution...
        Unresolved
    };

    // Used to store critical values combined with
    // comparison results and indexes
    struct criticalValueResult{
        double criticalValue;
        cmp_res compareResult;
        int index;
    };
}

#endif