//
// Created by zeb on 06/04/2022.
//
#include "ps_interfaces.hpp"

#ifndef LINEARFUNCTION_HPP
#define LINEARFUNCTION_HPP


namespace ps_framework {
    class LinearFunction{
    public:
        LinearFunction(double a, double b) : a(a), b(b) {}
        double a;
        double b;
        double compute(double x){
            return a*x + b;
        }
        double getRoot(){
            return -b/a;
        }
    };

    class LinearFunctionComparer : ps_framework::IComparer<LinearFunction> {
        double getCriticalValue(LinearFunction*, LinearFunction*);
        cmp_res getCompareResult(LinearFunction*, LinearFunction*, cmp_res);
    };
}


#endif //LINEARFUNCTION_HPP
