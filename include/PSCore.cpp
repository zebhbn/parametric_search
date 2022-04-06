//
// Created by zeb on 06/04/2022.
//

#include "PSCore.hpp"
#include <algorithm> // Used in binary search to find median
#include <limits>

ps_framework::PSCore::PSCore(ISeqAlgo *seqAlgo) : seqAlgo(seqAlgo) {
    // Set the searching interval to [-infinity, infinity]
    start = - std::numeric_limits<double>::infinity();
    end = std::numeric_limits<double>::infinity();
}

bool ps_framework::PSCore::isInInterval(double x) {
    return (x>start && x<end);
}

ps_framework::cmp_res ps_framework::PSCore::compareToLambdaStar(double x) {
    // Check if value is outside searching interval
    if (isInInterval(x)){
        if (x<start) {return LessThan;}
        else if (x>end) {return GreaterThan;}
        else {return EqualTo;}
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

void ps_framework::PSCore::binarySearch(std::vector<ps_framework::criticalValueResult> * vec) {
    // Initialize binary searching interval
    int e = 0;
    int f = vec->size() - 1;

    while (e<f) {
        int m = e + f/2;
        // Sort just enough to find median
        std::nth_element(vec->begin()+e, vec->begin()+m, vec->begin()+f,
                         [this](auto e1, auto e2){ return e1.criticalValue<e2.criticalValue; });
        // Get median
        auto mElm = (*vec)[m];

        // Compare to lambda*
        auto res = compareToLambdaStar(mElm.criticalValue);

        if (res == LessThan || res == EqualTo)
            e = m;
        else
            f = m;
    }
}

void ps_framework::PSCore::runCompareList(std::vector<ps_framework::criticalValueResult> *vec) {
    // Run binary search
    binarySearch(vec);

    // Run through vector and store cmp results
    for (auto e : *vec){
        auto res = compareToLambdaStar(e.criticalValue);
        e.compareResult = res;
    }
}
