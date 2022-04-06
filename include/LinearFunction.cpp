//
// Created by zeb on 06/04/2022.
//

#include "LinearFunction.hpp"
#include <limits>

double ps_framework::LinearFunctionComparer::getCriticalValue(LinearFunction * f1, LinearFunction *f2) {
    // If no intersection then compare b values of lines
    if (f1->a == f2->a)
        return std::numeric_limits<double>::quiet_NaN();
    // Get root of f1-f2
    double root = ((f2->b)-(f1->b))/((f1->a)-(f2->a));
    // The root is the critical value
    return root;
}

ps_framework::cmp_res ps_framework::LinearFunctionComparer::
    getCompareResult(LinearFunction *f1, LinearFunction *f2, cmp_res cmpRes) {
        if (cmpRes == LessThan || cmpRes == EqualTo) {
            // the intersection is not in the searching interval
            // and is in the lower end of the interval
            // So we should return depending on which one has the
            // smallest slope
            if ((f1->a) < (f2->a))
                return LessThan;
            else
                return GreaterThan;
        }
        else /*if (root >= end)*/ {
            if ((f1->a) > (f2->a))
                return LessThan;
            else
                return GreaterThan;
        }
}