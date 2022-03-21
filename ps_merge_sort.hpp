#include "ps_framework.hpp"
#include <vector>

#ifndef P_MERGE_SORT
#define P_MERGE_SORT


class PSMergeSort{
    public:
        PSMergeSort(PSFramework *ps_fw, std::vector<FunctionBase> *fs);
        void setFunctionVector(std::vector<FunctionBase> *fs);
        void sort();
    private:
        PSFramework *ps_inst;
        std::vector<FunctionBase> *funcs;
        void merge(int p, int q, int r);
        void mergeSort(int l, int r);
        
};
#endif