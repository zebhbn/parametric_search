//
// Created by zeb on 06/04/2022.
//

#include "LinearFunction.hpp"
#include <limits>
#include <cmath>

double ps_framework::LinearFunctionComparer::getCriticalValue(LinearFunction * f1, LinearFunction *f2) {
    // If no intersection then compare b values of lines
    // Here we set the root/critical value as not a number
    if (f1->a == f2->a){
        return std::numeric_limits<double>::quiet_NaN();
    }
    // Get root of f1-f2
    double root = ((f2->b)-(f1->b))/((f1->a)-(f2->a));
    // The root is the critical value
    return root;
}

ps_framework::cmp_res ps_framework::LinearFunctionComparer::
    getCompareResult(LinearFunction *f1, LinearFunction *f2, cmp_res cmpRes) {
        // root was nan i.e. no intersection between lines
        if (cmpRes == Unresolved) {
            // If no intersection then compare b values of lines
            if (f1->b < f2->b)
                return LessThan;
            else
                return GreaterThan;
        }
        else if (cmpRes == EqualTo) {
            return cmpRes;
        }
        else if (cmpRes == LessThan) {
            if ((f1->a) > (f2->a)) {
                return GreaterThan;
            }
            else{
                return LessThan;
            }
        }
        else /*if (root >= end)*/ {
            if ((f1->a) > (f2->a))
                return LessThan;
            else
                return GreaterThan;
        }
}

ps_framework::cmp_res ps_framework::LinearFunctionComparer::getCompareResult(LinearFunction f1, LinearFunction f2, cmp_res cmpRes) {
    return getCompareResult(&f1,&f2,cmpRes);
}

double ps_framework::LinearFunctionComparer::getCriticalValue(LinearFunction f1, LinearFunction f2) {
    return getCriticalValue(&f1, &f2);
}
