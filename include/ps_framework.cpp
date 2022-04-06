#include "ps_framework.hpp"
#include <limits>
#include <iostream>
#include <cassert>


//PSFramework::PSFramework(){
//    // Set the searching interval to [-infty,infty]
//    start = - std::numeric_limits<double>::infinity();
//    end = std::numeric_limits<double>::infinity();
//}
// Use if there are some predefined interval
template <typename T>
ps_framework::PSFramework<T>::PSFramework(
        IParAlgo* parAlgo,
        IComparer<T>* comparer,
        ISeqAlgo* seqAlgo,
        ISchedular* schedular,
        IPSCore* psCore,
        double start,
        double end
) :
        parAlgo(parAlgo),
        comparer(comparer),
        seqAlgo(seqAlgo),
        schedular(schedular),
        psCore(psCore),
        start(start),
        end(end)
{}

template <typename T>
ps_framework::PSFramework<T>::PSFramework(
        IParAlgo* parAlgo,
        IComparer<T>* comparer,
        ISeqAlgo* seqAlgo,
        ISchedular* schedular,
        IPSCore* psCore
    ) :
        parAlgo(parAlgo),
        comparer(comparer),
        seqAlgo(seqAlgo),
        schedular(schedular),
        psCore(psCore)
        {
    // Set the searching interval to [-infty,infty]
    start = - std::numeric_limits<double>::infinity();
    end = std::numeric_limits<double>::infinity();

}

// For testing purposes
template <typename T>
ps_framework::PSFramework<T>::PSFramework(){};

//void PSFramework::setSeqAlgo(ISeqAlgo *sa){
//    seqAlgo = sa;
//}

template <typename T>
bool ps_framework::PSFramework<T>::isInInterval(double x){
    return (x>start && x<end);
}

template <typename T>
void ps_framework::PSFramework<T>::spawn(co_task<void> *task) {
    schedular->spawn(task);
}

template <typename T>
void ps_framework::PSFramework<T>::compare(T *, T *, cmp_res *) {
    // TODO: implement this
    assert(false);
}

//template <typename T>
//void ps_framework::PSFramework<T>::resolveComparisons() {
//    // TODO: implement this
//    assert(false);
//}

// The value returned means the following
// when comparing the functions at lambda*
// return -1 if f1 < f2
// return 1 if f1 > f1
// return 0 if f1==f2
//int PSFramework::compare(FunctionBase f1, FunctionBase f2) {
//
//    // If no intersection then compare b values of lines
//    if (f1.a == f2.a){
//        if (f1.b < f2.b)
//            return -1;
//        else
//            return 1;
//    }
//
//    // create new function f1-f2
//    // FunctionBase ff = f1-f2;
//    // get root of ff
//    // double root = ff.getRoot();
//    double root = (f2.b-f1.b)/(f1.a-f2.a);
//    // check if root is in the searching interval
//    if(isInInterval(root)){
//        // No short cuts, use the sequential algorithm
//        // Here we should put off work untill absolutly nessecary
//        int sign = seqAlgo->compare(root);
//        // Update searching interval with newly percieved information
//        if (sign == 0){
//            start = root;
//            end = root;
//            // std::cout<<"start="<<start<<" end="<<end<<std::endl;
//        } else if (sign == -1){
//            start = root;
//            // std::cout<<"start="<<start<<std::endl;
//            if (f1.a > f2.a)
//                sign = 1;
//            else
//                sign = -1;
//        } else {
//            assert(sign==1);
//            end = root;
//            // std::cout<<"end="<<end<<std::endl;
//            if (f1.a < f2.a)
//                sign = 1;
//            else
//                sign = -1;
//        }
//        // std::cout<<"start="<<start<<" end="<<end<<std::endl;
//        return sign;
//    } else {
//        // std::cout << "hit outside search interval" << std::endl;
//        // std::cout << "root="<<root << std::endl;
//        // std::cout<<"f1: a="<<f1.a<<" b="<<f1.b<<std::endl;
//        // std::cout<<"f2: a="<<f2.a<<" b="<<f2.b<<std::endl;
//        if (root <= start) {
//            // the intersection is not in the searching interval
//            // and is in the lower end of the interval
//            // So we should return depending on which one has the
//            // smallest slope
//            if (f1.a < f2.a)
//                return -1;
//            else
//                return 1;
//        }
//        else /*if (root >= end)*/ {
//            if (f1.a > f2.a)
//                return -1;
//            else
//                return 1;
//        }
//    }
//}




