// #include <iostream>
#include "ps_merge_sort.hpp"
#include "ps_framework.hpp"
#include <vector>

// using namespace std;

// The two function merge and mergesort are based
// https://www.programiz.com/dsa/merge-sort

// PSMergeSort::PSMergeSort(PSFramework *ps_fm, IProblemInstance *ip){
//     ps_inst = ps_fm;
//     p_inst = ip;
// }

PSMergeSort::PSMergeSort(PSFramework *ps_fm, std::vector<FunctionBase> *fs) {
    ps_inst = ps_fm;
    funcs = fs;
}

void PSMergeSort::setFunctionVector(std::vector<FunctionBase> *fs) {
    funcs = fs;
}


// Merge two subarrays L and M into arr
void PSMergeSort::merge(int p, int q, int r) {

    // Create L ← A[p..q] and M ← A[q+1..r]
    int n1 = q - p + 1;
    int n2 = r - q;

    std::vector<FunctionBase>  L(n1), M(n2);

    for (int i = 0; i < n1; i++)
        L[i] = (*funcs)[p + i];
    for (int j = 0; j < n2; j++)
        M[j] = (*funcs)[q + 1 + j];

    // Maintain current index of sub-arrays and main array
    int i, j, k;
    i = 0;
    j = 0;
    k = p;

    // Until we reach either end of either L or M, pick larger among
    // elements L and M and place them in the correct position at A[p..r]
    while (i < n1 && j < n2) {
        int sign = ps_inst->compare(L[i], M[j]);
        if (sign==-1 || sign==0) {
            (*funcs)[k] = L[i];
            i++;
        } else {
            (*funcs)[k] = M[j];
            j++;
        }
        k++;
    }

    // When we run out of elements in either L or M,
    // pick up the remaining elements and put in A[p..r]
    while (i < n1) {
        (*funcs)[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        (*funcs)[k] = M[j];
        j++;
        k++;
    }
}

// Divide the array into two subarrays, sort them and merge them
void PSMergeSort::mergeSort(int l, int r) {
  if (l < r) {
    // m is the point where the array is divided into two subarrays
    int m = l + (r - l) / 2;

    mergeSort(l, m);
    mergeSort(m + 1, r);

    // Merge the sorted subarrays
    merge(l, m, r);
  }
}


// The external call to create a sorted copy of p_inst
void PSMergeSort::sort(){
    int n = (*funcs).size();
    int l = 0;
    int r = n-1;
    mergeSort(l,r);
    return;
}

// void display_arr(int arr[], int n){
//     for (int i=0;i<100; i++) {
//         cout << arr[i] << " ";
//     }
//     cout << endl;
// }


// int main(){
//     int n = 100;
//     int arr[n];
//     for (int i=0; i<100;i++){
//         arr[i] = 100-i;
//     }
//     cout << "Unsorted array" << endl;
//     display_arr(arr,n);
//     mergeSort(arr,0,99);
//     display_arr(arr,n);
// }

