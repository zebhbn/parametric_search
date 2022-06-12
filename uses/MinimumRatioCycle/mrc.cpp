//
// Created by zeb on 03/06/2022.
//


#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <optional>
#include "LinearFunction.hpp"
#include "GraphGenerators.hpp"
#include "MultiTScheduler.hpp"
#include "MultiTComparisonResolver.hpp"
#include <mutex>
#include <condition_variable>
#include "PSFloyd.hpp"


void test() {
//    auto graphVectorMat = genAllGraphs();
//    auto graphVectorU = genAllGraphs();
    auto graph = generateSimpleGraph();
    auto graph2 = generateSimpleGraph();
    auto floyd = MultiPSFloyd(graph, graph2);
    std::cout << "Normal version : n=" << graph->size() <<  std::endl;
    floyd.numThreads = 8;
    floyd.run();
//    while (!graphVectorMat->empty()) {
//        auto mat = graphVectorMat->front();
//        auto u = graphVectorU->front();
//        auto mat = graphVectorMat;
//        auto u = graphVectorU;
//        std::cout << "Normal version : n=" << graphVectorU->front()->size() <<  std::endl;
//        auto floyd = PSFloyd(mat, u);
//        floyd.run();

//        int numThreads = 8;
//        std::cout << "Multi threaded naive version : n=" << u->size() << " : " << numThreads << " threads" << std::endl;
//        auto floyd = MultiPSFloyd(mat, u);
//        floyd.numThreads = numThreads;
//        floyd.run();

//        graphVectorMat->pop();
//        graphVectorU->pop();
//    }
}

int main(){
    test();
    exit(0);
}

