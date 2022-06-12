//
// Created by zeb on 06/04/2022.
//

#ifndef MEDIANOFLINES_PSCORE_HPP
#define MEDIANOFLINES_PSCORE_HPP

#include "PSInterfaces.hpp"
#include <cmath>
#include <algorithm>

namespace ps_framework {
    class PSCore : public ps_framework::IPSCore {
    public:
        PSCore(ISeqAlgo*);
        PSCore(ISeqAlgo*, double, double);
        void runCompareList(std::vector<ps_framework::CmpCvResult> *);
        double start;
        double end;
        bool isInInterval(double);
        CmpRes compareToLambdaStar(double);
    private:
        ISeqAlgo *seqAlgo;
        void binarySearch(std::vector<ps_framework::CmpCvResult> *);
    };
}

ps_framework::PSCore::PSCore(ISeqAlgo *seqAlgo) : seqAlgo(seqAlgo) {
    // Set the searching interval to [-infinity, infinity]
    start = - std::numeric_limits<double>::infinity();
    end = std::numeric_limits<double>::infinity();
}

bool ps_framework::PSCore::isInInterval(double x) {
    return (x>start && x<end);
}

ps_framework::CmpRes ps_framework::PSCore::compareToLambdaStar(double x) {
//    if(std::isnan(x)){
//        return Unresolved;
//    }
    // Check if value is outside searching interval
//    else if (!isInInterval(x)){
    if (!isInInterval(x)){
            if (x<=start) {return LessThan;}
            else if (x>=end) {return GreaterThan;}
            else {assert(false);}
    }

    // We have to do it the hard way now
    auto res = seqAlgo->compare(x);
    // Restrict searching interval according to result
    if (res == LessThan) {start = x;}
    else if (res == GreaterThan) {end = x;}
    else {start = x; end = x;}
    return res;
}

ps_framework::PSCore::PSCore(ISeqAlgo *seqAlgo, double start, double end) :
        seqAlgo(seqAlgo), start(start), end(end) {}

void ps_framework::PSCore::binarySearch(std::vector<ps_framework::CmpCvResult> * vec) {
    // Initialize binary searching interval
    int e = 0;
    int f = vec->size() - 1;
    // Sort the values in the vector
    // Could also just use nth_element on every searching half of the binary search
    std::sort(vec->begin(), vec->end(),
                     [this](auto e1, auto e2){ return e1.criticalValue<e2.criticalValue; });

    while (e<f) {
        int m = e + f/2;
        if ((m == e) || (m == f)){
            break;
        }
        // Sort just enough to find median
//        std::nth_element(vec->begin()+e, vec->begin()+m, vec->begin()+f,
//                         [this](auto e1, auto e2){ return e1.criticalValue<e2.criticalValue; });
        // Get median
        auto mElm = (*vec)[m];

        // Compare to lambda*
        auto res = compareToLambdaStar(mElm.criticalValue);

        // If equal to then just stop the binary search
        if (res == EqualTo) {return;}
        if (res == LessThan)
            e = m;
        else
            f = m;
    }
}

void ps_framework::PSCore::runCompareList(std::vector<CmpCvResult> *vec) {
    // Run binary search
    binarySearch(vec);

    // Run through vector and store cmp results
    for (auto &e : *vec){
        auto res = compareToLambdaStar(e.criticalValue);
        e.compareResult = res;
    }
}


#endif //MEDIANOFLINES_PSCORE_HPP
