//
// Created by zeb on 09/06/2022.
//

#include "ComparisonResolver.hpp"

#ifndef MINIMUMMEANCYCLE_DIRECTCOMPARISONRESOLVER_HPP
#define MINIMUMMEANCYCLE_DIRECTCOMPARISONRESOLVER_HPP

namespace ps_framework{
    template<typename T>
    class DirectComparisonResolver : public ComparisonResolver<T> {
    public:
        DirectComparisonResolver(Scheduler *s, IPSCore *core, IComparer<T> *c) : ComparisonResolver<T>(s,core,c) {};
//        using ComparisonResolver<T>::compare;
        using ComparisonResolver<T>::iComparer;
        using ComparisonResolver<T>::psCore;
        await_data preCompare(T t1, T t2);
    };
}

template <typename T>
ps_framework::await_data ps_framework::DirectComparisonResolver<T>::preCompare(T t1, T t2) {
    CmpRes directRes = Unresolved;
    // Compute critival value
    double cv = iComparer->getCriticalValue(t1, t2);
    if (std::isnan(cv)) {
        directRes = iComparer->getCompareResult(t1,t2,Unresolved);
    } else if (!(psCore->isInInterval(cv))) {
        directRes =  iComparer->getCompareResult(
                t1,
                t2,
                psCore->compareToLambdaStar(cv)
        );
    } else {
        return ComparisonResolver<T>::preCompare(t1,t2);
    }
    return await_data{0, directRes};
}

#endif //MINIMUMMEANCYCLE_DIRECTCOMPARISONRESOLVER_HPP
