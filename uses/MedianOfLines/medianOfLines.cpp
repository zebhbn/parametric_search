//
// Created by zeb on 03/06/2022.
//
#include <vector>
#include "PSInterfaces.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include "PSCore.hpp"
#include "PSQuicksort.hpp"
#include "LinearFunction.hpp"
#include <chrono>
#include <cmath>



class SeqAlgoMedianLines : public ps_framework::ISeqAlgo {
public:
    SeqAlgoMedianLines(std::vector<ps_framework::LinearFunction> *fs){
        p_funcs = fs;
    }
    ps_framework::CmpRes compare(double lambda){
        // Map f(lambda) -> double
        int n = (*p_funcs).size();
        std::vector<double> vals(n);
        for (int i = 0; i<n;i++)
            vals[i] = (*p_funcs)[i].compute(lambda);
        // Get median and return sign of result
        std::nth_element(vals.begin(), vals.begin() + (n/2),vals.end());
        double median = vals[n/2];
        if(median==0){
            return ps_framework::EqualTo;
        } else if (median<0){
            return ps_framework::LessThan;
        } else {
            return ps_framework::GreaterThan;
        }
    }
private:
    std::vector<ps_framework::LinearFunction> *p_funcs;
};



double psVersion(int numLines) {
    // Generate lines
    int number_of_lines = numLines;
    std::vector<ps_framework::LinearFunction> lines;
    lines.reserve(number_of_lines);
    int a = 1;
    int b = 0;
    for (size_t i=0; i<number_of_lines; ++i){
        a = (a*i) % number_of_lines + 1;
        b = (b*i) % number_of_lines - a;
        lines.push_back(ps_framework::LinearFunction((double)a,(double)b));
    }
    // Instantiate everything
    SeqAlgoMedianLines seqAlgo = SeqAlgoMedianLines(&lines);
    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
    auto scheduler = ps_framework::Scheduler();
    auto comparisonResolver = ps_framework::ComparisonResolver<ps_framework::LinearFunction>(
            &scheduler,
            &psCore,
            &linComparer);
    auto quickSort = ps_framework::PSQuicksort<ps_framework::LinearFunction>(&scheduler, &comparisonResolver, &lines);

    std::cout<<"Running quicksort with n = "<<number_of_lines<<std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    quickSort.sort();
    ps_framework::LinearFunction median = lines[number_of_lines/2];


    // Run and measure time
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);
    auto res = seqAlgo.compare(median.getRoot());

    std::cout << "    lambda* = "
              << (median.getRoot())
              << std::endl
              << "    time    = "
              << duration
              << std::endl;

    return median.getRoot();
}

int main(){
    int numLines = 101;
//    for (int i = 7; i < 17; i++) {
//        psVersion(std::pow(2,i));
//    }
    double lambda_star = psVersion(numLines);
    return 0;
}
