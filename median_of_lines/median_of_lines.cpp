//#include "ps_merge_sort.hpp"

//#include "ps_framework.hpp"
#include "ps_quicksort.hpp"
#include "LinearFunction.hpp"
#include "schedular.hpp"
#include "PSCore.hpp"
#include <vector>
#include <algorithm> // sort: used to find median in A_s algo, could ude nth_element
#include <iostream> // Write result to stdout
#include <cassert>

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

//double test(int numLines) {
//    // Generate lines
//    int number_of_lines = numLines;
//    std::vector<FunctionBase> lines;
//    std::vector<FunctionBase> sorted_lines;
//    lines.reserve(number_of_lines);
//    sorted_lines.reserve(number_of_lines);
//    int a = 1;
//    int b = 0;
//    for (size_t i=0; i<number_of_lines; ++i){
//        a = (a*i) % number_of_lines + 1;
//        b = (b*i) % number_of_lines - a;
//        lines.push_back(FunctionBase((double)a,(double)b));
//        sorted_lines.push_back(FunctionBase((double)a,(double)b));
//    }
//    // Instantiate everything
//    PSFramework fm = PSFramework();
//    SeqAlgoMedianLines seqAlgo = SeqAlgoMedianLines(&sorted_lines);
//    fm.setSeqAlgo(&seqAlgo);
//    PSMergeSort mSort = PSMergeSort(&fm, &sorted_lines);
//    // Sort the lines using parametric search based merge sort
//    mSort.sort();
//    // Lambda star is now the root of the median line
//    double lambda_star = sorted_lines[number_of_lines/2].getRoot();
//    double root,root2;
//    for (int i=0; i<number_of_lines;i++) {
//        root = sorted_lines[i].getRoot();
//        // root = lines[i].getRoot();
//        // assert(root==root2);
//        // std::cout<<root<<std::endl;
//        // int bob = seqAlgo.compare(root);
//        // std::cout<<bob<<std::endl;
//        // if (root > -0.03 && root < 0.03)
//        //     std::cout << "root=" << root << " i=" << i << std::endl;
//
//    }
//    return lambda_star;
//}

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
        // std::cout << "a=" << a << " b=" << b << std::endl;
//        lines.push_back(FunctionBase((double)a,(double)b));

        lines.push_back(ps_framework::LinearFunction((double)a,(double)b));
    }
    // Instantiate everything
//    PSFramework fm = PSFramework();
    SeqAlgoMedianLines seqAlgo = SeqAlgoMedianLines(&lines);
//    fm.setSeqAlgo(&seqAlgo);
    // PSMergeSort mSort = PSMergeSort(&fm, &lines);
    // Sort the lines using parametric search based merge sort
    // mSort.sort();
    // Lambda star is now the root of the median line
    double star = 123123123;
    for (int i = 0; i<number_of_lines;i++){
        if (seqAlgo.compare(lines[i].getRoot())==0)
            star = lines[i].getRoot();
    }
    return star;
}



int main(){
    int numLines = 101;
//    double lambda_star = test(numLines);
    double test_lambda_star = nonPsVersion(numLines);
//    // Output result
//    std::cout << "lambda* = " << lambda_star << std::endl;
    std::cout << "real lambda* = " << test_lambda_star << std::endl;


//    auto vec = std::vector<int>();
//    auto psFrame = new ps_framework::PSFramework<int>();
//    auto psQuicksort = new ps_framework::PSQuicksort<int>(psFrame, &vec);

    return 0;
}