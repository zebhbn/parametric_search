#include "ps_quicksort.hpp"
#include "ps_interfaces.hpp"
#include "LinearFunction.hpp"
#include "schedular.hpp"
#include "PSCore.hpp"
#include <vector>
#include <algorithm> // sort: used to find median in A_s algo, could ude nth_element
#include <iostream> // Write result to stdout
#include <cassert>
#include "MultiTSchedular.hpp"

class SeqAlgoMedianLines : public ps_framework::ISeqAlgo {
    public:
        SeqAlgoMedianLines(std::vector<ps_framework::LinearFunction> *fs){
            p_funcs = fs;
        }
        ps_framework::cmp_res compare(double lambda){
            // Map f(lambda) -> double
            int n = (*p_funcs).size();
            std::vector<double> vals(n);
            for (int i = 0; i<n;i++)
                vals[i] = (*p_funcs)[i].compute(lambda);
            // Get median and return sign of result
            sort(vals.begin(),vals.end());
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


double nonPsVersion(int numLines) {
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
    SeqAlgoMedianLines seqAlgo = SeqAlgoMedianLines(&lines);
    double star = 123123123;
    for (int i = 0; i<number_of_lines;i++){
        if (seqAlgo.compare(lines[i].getRoot())==ps_framework::EqualTo)
            star = lines[i].getRoot();
    }
    return star;
}
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
    auto schedular = ps_framework::Schedular<ps_framework::LinearFunction>(&psCore, &linComparer);
    auto quickSort = ps_framework::PSQuicksort<ps_framework::LinearFunction>(&schedular, &lines);
    quickSort.sort();
    ps_framework::LinearFunction median = lines[number_of_lines/2];
    return median.getRoot();
}
double psMultiThreadVersion(int numLines) {
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
    auto schedular = ps_framework::MultiThreadSchedular<ps_framework::LinearFunction>(&psCore, &linComparer,8);
    auto quickSort = ps_framework::PSQuicksort<ps_framework::LinearFunction>(&schedular, &lines);
    quickSort.sort();
    ps_framework::LinearFunction median = lines[number_of_lines/2];
    return median.getRoot();
}

int main(){
    int numLines = 101;
    double lambda_star = psVersion(numLines);
    double lambda_star_multi = psMultiThreadVersion(numLines);
    double test_lambda_star = nonPsVersion(numLines);
    // Output result
    std::cout << "lambda* = " << lambda_star << std::endl;
    std::cout << "multilambda* = " << lambda_star_multi << std::endl;
    std::cout << "real lambda* = " << test_lambda_star << std::endl;
    return 0;
}