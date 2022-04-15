#include "ps_interfaces.hpp"
#include "co_task.hpp"
#include <vector>

#ifndef PS_QUICKSORT
#define PS_QUICKSORT

namespace ps_framework {
    template<typename T>
    class PSQuicksort {
    public:
        PSQuicksort(
//                PSFramework<T> *psfw,
                ISchedular<T> *schedular,
                std::vector<T> *vec
        );
        ~PSQuicksort();

        // For testing purposes
//        PSQuicksort();

        void sort();

    private:
        co_task<int> partition(int, int);

        co_task_void quicksort(int, int);

        void swap(int, int);

//        PSFramework<T> *psframe;
        ISchedular<T> *schedular;
        std::vector<T> *arr;
        std::vector<cmp_res> *res;
    };
}

// Quicksort inspired by https://www.geeksforgeeks.org/quick-sort/

template <typename T>
ps_framework::PSQuicksort<T>::PSQuicksort(ISchedular<T> *schl, std::vector<T> *vec){
    arr = vec;
    schedular = schl;
    // Initialize the result array
    res = new std::vector<cmp_res>();
    res->reserve(arr->size());
}

template <typename T>
void ps_framework::PSQuicksort<T>::sort() {
    co_task_void initTask = quicksort(0, arr->size()-1);
    schedular->spawn(&initTask);
    schedular->run();
}

// For testing purposes
//template <typename T>
//ps_framework::PSQuicksort<T>::PSQuicksort() {};

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
    T pivot = (*arr)[high];

    int i = (low-1);

    for (int j = low; j <= high - 1; j++) {
        schedular->addComparison(&((*arr)[j]), &pivot, &(*res)[j]);
    }
    // All comparisons have been requested
    // return control
    co_await std::suspend_always();

    // Comparisons have been resolved
    for (int j = low; j <= high - 1; j++) {
        if (((*res)[j] == LessThan)) {
//        if (((*res)[j] == LessThan )
//            || ((*res)[j] == EqualTo)) {
            i++;
            swap(i, j);
        }
    }

    swap(i+1, high);
    co_return (i+1);
};

template <typename T>
ps_framework::co_task_void ps_framework::PSQuicksort<T>::quicksort(int low, int high){
//    std::cout<<"quicksort task started: ["<<low<<", "<<high<<"]"<< std::endl;
    if (low < high) {

        // Create the partition task
        co_task<int> partitionTask = partition(low, high);
        // Start the task
        partitionTask.resume();
        // partition is now exhausted for comparisons
        // and needs answers to go further
        // Therefore we suspend this coroutine and wait...
        co_await std::suspend_always();

        // All comparisons have now been resolved and so we
        // resume partitionTask
        partitionTask.resume();

        // Partition should now be finished and we retrieve the
        // pivot index

        int pivot = partitionTask.result();
        partitionTask.destroyMe();

        // Make the recursive calls by calling spawning them as tasks
        // in the schedular
        if (low < (pivot+1)) {
//            co_task_void recTask1 = quicksort(low, pivot - 1);
            auto recTask1 = new co_task_void(quicksort(low,pivot-1));
            schedular->spawn(recTask1);
        }
        if ((pivot+1) < high) {
//            co_task_void recTask2 = quicksort(pivot + 1, high);
            auto recTask2 = new co_task_void(quicksort(pivot+1,high));
            schedular->spawn(recTask2);

        }
    }
//    std::cout<< "finishing quicksort coroutine"<<std::endl;
    co_return;
}

template<typename T>
ps_framework::PSQuicksort<T>::~PSQuicksort() {
    // Cleanup
    res->clear();
    delete res;
};

#endif