//
// Created by zeb on 15/04/2022.
//

#include "ps_interfaces.hpp"
#include <cassert>

// This should amount to a negative cycle detection algorithm
class SeqAlgoMinimumMeanCycle : public ps_framework::ISeqAlgo{
    ps_framework::cmp_res compare(double lambda){
        assert(false);
    }
};

double psVersion(){
    assert(false);
    // Following approach 1 in megiddo 83'

    // 1. Start by creating comparison between each ordered pair
    // (i,j)
    // 2. Then using a new comparerer and object definition generate
    // O(V³) breakpoints. This will probably require a modification
    // to the current framework...
    // 3. Use binary search/PSCore with negative cycle detection
    // algorithm used as sequential algorithm ISeqAlgo.
}

// This could maybe be Karps algorithm...
double nonPsVersion(){
    assert(false);
}

int main(){
    return 0;
}
