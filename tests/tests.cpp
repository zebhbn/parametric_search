//
// Created by zeb on 08/06/2022.
//

#include "PSFloyd.hpp"

int main() {
    auto mat = generateBigGraphOpt();
    auto u = generateBigGraphOpt();
    auto floyd = PSFloyd(&mat, &u);
    floyd.run()
}

