//
// Created by zeb on 06/04/2022.
//
#include "PSInterfaces.hpp"

#ifndef LINEARFUNCTION_HPP
#define LINEARFUNCTION_HPP


namespace ps_framework {
    class LinearFunction{
    public:
        LinearFunction(double a, double b) : a(a), b(b) {}
        LinearFunction() : a(0), b(0) {};
        double a;
        double b;
        double compute(double x){
            return a*x + b;
        }
        double getRoot(){
            return -b/a;
        }
        LinearFunction operator+ (const LinearFunction& func){
            LinearFunction newFunc(0,0);
            newFunc.a = this->a + func.a;
            newFunc.b = this->b + func.b;
            return newFunc;
        };
        LinearFunction operator- (const LinearFunction& func){
            LinearFunction newFunc(0,0);
            newFunc.a = this->a - func.a;
            newFunc.b = this->b - func.b;
            return newFunc;
        };
        bool operator== (const LinearFunction& func){
            return ((this->a==func.a)&&(this->b==func.b));
        };
        // String representation
        friend std::ostream& operator<<(std::ostream &s, const LinearFunction &point) {
            return s << "(" << point.a << "*x, " << point.b << ")";
        }
        // Copy constructor
        LinearFunction(const LinearFunction& fn)
        {
            a = fn.a;
            b = fn.b;
        }
    };

    class LinearFunctionComparer : public ps_framework::IComparer<LinearFunction> {
        double getCriticalValue(LinearFunction*, LinearFunction*);
        double getCriticalValue(LinearFunction, LinearFunction);
        CmpRes getCompareResult(LinearFunction*, LinearFunction*, CmpRes);
        CmpRes getCompareResult(LinearFunction, LinearFunction, CmpRes);
    };
}


#endif //LINEARFUNCTION_HPP
