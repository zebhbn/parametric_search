//
// Created by zeb on 05/04/2022.
//

#include "ps_quicksort.hpp"
#include <vector>

// Quicksort inspired by https://www.geeksforgeeks.org/quick-sort/

template <typename T>
ps_framework::PSQuicksort<T>::PSQuicksort(PSFramework<T> *psfw, std::vector<T> *vec){
    arr = vec;
    psframe = psfw;
    // Initialize the result array
    res = new std::vector<cmp_res>();
    res->reserve(arr->size());
}

// For testing purposes
template <typename T>
ps_framework::PSQuicksort<T>::PSQuicksort() {};

template <typename T>
void ps_framework::PSQuicksort<T>::swap(int i, int j) {
    // Should have mutex for multithreading
    T tmp = (*arr)[i];
    (*arr)[i] = (*arr)[j];
    (*arr)[j] = tmp;
}

template <typename T>
ps_framework::co_task<int> ps_framework::PSQuicksort<T>::partition(int low, int high){
    // Pivot should maybe be selected randomly
    T pivot = arr[high];

    int i = (low-1);

    for (int j = low; j <= high; j++) {
        psframe->compare(&arr[j], &pivot, &res[j]);
    }
    // All comparisons have been requested
    // return control
    co_await std::suspend_always();

    // Comparisons have been resolved
    for (int j = low; j <= high; j++) {
        if (((*res)[j] == LessThan )
        || ((*res)[j] == EqualTo)) {
            i++;
            swap(i, j);
        }
    }

    swap(i+1, high);
    co_return (i+1);
};

template <typename T>
ps_framework::co_task<void> ps_framework::PSQuicksort<T>::quicksort(int low, int high){
    if (low < high) {
        // Create the partition task
        co_task<int> partitionTask = partition(arr, low, high);
        // Start the task
        partitionTask.resume();
        // partition is now exhausted for comparisons
        // and needs answers to go further
        // Therefore we suspend this coroutine and wait...
        co_await std::suspend_always();

        // All comparisons have now been resolved and so we
        // resume partitionTask
        partitionTask.resume();

        // Partition should now be finished and we retrive the
        // pivot index
        int pivot = partitionTask.result();

        // Make the recursive calls by calling spawning them as tasks
        // in the schedular
        co_task<void> recTask1 = quicksort(arr, low, pivot - 1);
        co_task<void> recTask2 = quicksort(arr, pivot + 1, high);

        // Spawn them both
        psframe->spawn(recTask1);
        psframe->spawn(recTask2);

    }

    co_return;
};

