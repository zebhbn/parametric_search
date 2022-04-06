//
// Created by zeb on 06/04/2022.
//

#ifndef MEDIANOFLINES_PSCORE_HPP
#define MEDIANOFLINES_PSCORE_HPP

#include "ps_interfaces.hpp"

namespace ps_framework {
    class PSCore : ps_framework::IPSCore {
    public:
        PSCore(ISeqAlgo*);
        PSCore(ISeqAlgo*, double, double);
        void runCompareList(std::vector<ps_framework::criticalValueResult> *);
    private:
        double start;
        double end;
        ISeqAlgo *seqAlgo;
        bool isInInterval(double);
        cmp_res compareToLambdaStar(double);
        void binarySearch(std::vector<ps_framework::criticalValueResult> *);
    };
}


#endif //MEDIANOFLINES_PSCORE_HPP
