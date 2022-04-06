#include "ps_framework.hpp"
#include <limits>
#include <iostream>
#include <cassert>

double FunctionBase::getRoot(){return -b/a;};
double FunctionBase::compute(double l){return a*l + b;};
FunctionBase::FunctionBase(double x, double y){a=x; b=y;};
FunctionBase::FunctionBase(){a=0.0; b=0.0;};
FunctionBase FunctionBase::operator+ (const FunctionBase& func){
            FunctionBase newFunc;
            newFunc.a = this->a + func.a;
            newFunc.b = this->b + func.b;
            return newFunc;
};
FunctionBase FunctionBase::operator- (const FunctionBase& func){
            FunctionBase newFunc;
            newFunc.a = this->a - func.a;
            newFunc.b = this->b - func.b;
            return newFunc;
};
bool FunctionBase::operator== (const FunctionBase& func){
            return ((this->a==func.a)&&(this->b==func.b));
};
FunctionBase::FunctionBase(const FunctionBase &f1){
    a = f1.a;
    b = f1.b;
};

PSFramework::PSFramework(){
    // Set the searching interval to [-infty,infty] 
    start = - std::numeric_limits<double>::infinity();
    end = std::numeric_limits<double>::infinity();
}
// Use if there are some predefined interval
PSFramework::PSFramework(double e, double f){
    start = e;
    end = f;
}
void PSFramework::setSeqAlgo(ISeqAlgo *sa){
    seqAlgo = sa;
}

bool PSFramework::isInInterval(double x){
    return (x>start && x<end);
}
// The value returned means the following
// when comparing the functions at lambda*
// return -1 if f1 < f2
// return 1 if f1 > f1
// return 0 if f1==f2
int PSFramework::compare(FunctionBase f1, FunctionBase f2) {

    // If no intersection then compare b values of lines
    if (f1.a == f2.a){
        if (f1.b < f2.b)
            return -1;
        else
            return 1;
    }

    // create new function f1-f2
    // FunctionBase ff = f1-f2;
    // get root of ff
    // double root = ff.getRoot();
    double root = (f2.b-f1.b)/(f1.a-f2.a);
    // check if root is in the searching interval
    if(isInInterval(root)){
        // No short cuts, use the sequential algorithm
        // Here we should put off work untill absolutly nessecary
        int sign = seqAlgo->compare(root);
        // Update searching interval with newly percieved information
        if (sign == 0){
            start = root;
            end = root;
            // std::cout<<"start="<<start<<" end="<<end<<std::endl;
        } else if (sign == -1){
            start = root;
            // std::cout<<"start="<<start<<std::endl;
            if (f1.a > f2.a)
                sign = 1;
            else
                sign = -1;
        } else {
            assert(sign==1);
            end = root;
            // std::cout<<"end="<<end<<std::endl;
            if (f1.a < f2.a)
                sign = 1;
            else
                sign = -1;
        }
        // std::cout<<"start="<<start<<" end="<<end<<std::endl;
        return sign;
    } else {
        // std::cout << "hit outside search interval" << std::endl;
        // std::cout << "root="<<root << std::endl;
        // std::cout<<"f1: a="<<f1.a<<" b="<<f1.b<<std::endl;
        // std::cout<<"f2: a="<<f2.a<<" b="<<f2.b<<std::endl;
        if (root <= start) {
            // the intersection is not in the searching interval
            // and is in the lower end of the interval
            // So we should return depending on which one has the 
            // smallest slope
            if (f1.a < f2.a)
                return -1;
            else
                return 1;
        }
        else /*if (root >= end)*/ {
            if (f1.a > f2.a)
                return -1;
            else
                return 1;
        }
    }
}




