#include "PSInterfaces.hpp"
#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <vector>

#ifndef PS_QUICKSORT
#define PS_QUICKSORT

namespace ps_framework {
    template<typename T>
    class PSQuicksort {
    public:
        PSQuicksort(
//                PSFramework<T> *psfw,
                Scheduler *scheduler,
                ComparisonResolver<T> *comparisonResolver,
                std::vector<T> *vec
        );
        ~PSQuicksort();

        // For testing purposes
//        PSQuicksort();

        void sort();

    private:
//        co_task<int> partition(int, int);
        coroTaskVoid partition(int, int, int*);

        coroTaskVoid quicksort(int, int);

        void swap(int, int);
        coroTaskVoid cmp(ComparisonResolver<T> * cv, T elm, T pivot, int *pi, int j);

//        PSFramework<T> *psframe;
        Scheduler *scheduler;
        ComparisonResolver<T> *comparisonResolver;
        std::vector<T> *arr;

//        std::vector<cmp_res> *res;
    };
}

// Quicksort inspired by https://www.geeksforgeeks.org/quick-sort/

template <typename T>
ps_framework::PSQuicksort<T>::PSQuicksort(Scheduler *sched, ComparisonResolver<T> *cr, std::vector<T> *vec){
    arr = vec;
    scheduler = sched;
    comparisonResolver = cr;
    // Initialize the result array
//    res = new std::vector<cmp_res>();
//    res->reserve(arr->size());
}

template <typename T>
void ps_framework::PSQuicksort<T>::sort() {
    coroTaskVoid initTask = quicksort(0, arr->size()-1);
    scheduler->spawnIndependent(&initTask);
    scheduler->run();
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
ps_framework::coroTaskVoid ps_framework::PSQuicksort<T>::cmp(ComparisonResolver<T> * cv, T elm, T pivot, int *pi, int j) {
    auto res = co_await cv->compare(elm, pivot);
    if (res == LessThan) {
        (*pi)++;
        swap((*pi), j);
    }
    co_return;
}

template <typename T>
ps_framework::coroTaskVoid ps_framework::PSQuicksort<T>::partition(int low, int high, int *retval){
    // Pivot should maybe be selected randomly
    T pivot = (*arr)[high];

    int i = (low-1);

    for (int j = low; j <= high - 1; j++) {
//        schedular->addComparison(&((*arr)[j]), &pivot, &(*res)[j]);
        co_await scheduler->spawnDependent(new coroTaskVoid(
                [](ComparisonResolver<T> * cv, T elm, T pivot, int *pi, int j, auto qsObj) -> coroTaskVoid {
                    auto res = co_await cv->compare(elm, pivot);
                    if (res == LessThan) {
                        (*pi)++;
                        qsObj->swap((*pi),j);
                    }
                }(comparisonResolver, ((*arr)[j]), pivot, &i, j, this)
                ));
//        auto cmpTask = new coroTaskVoid(cmp(comparisonResolver, ((*arr)[j]), pivot, &i, j));
//        co_await scheduler->spawnDependent(cmpTask);
    }
//    // All comparisons have been requested
//    // return control
//    co_await std::suspend_always();
//
//    // Comparisons have been resolved
//    for (int j = low; j <= high - 1; j++) {
//        if (((*res)[j] == LessThan)) {
////        if (((*res)[j] == LessThan )
////            || ((*res)[j] == EqualTo)) {
//            i++;
//            swap(i, j);
//        }
//    }
    co_await std::suspend_always();

    swap(i+1, high);
    (*retval) = i+1;
    co_return;
};

template <typename T>
ps_framework::coroTaskVoid ps_framework::PSQuicksort<T>::quicksort(int low, int high){
//    std::cout<<"quicksort task started: ["<<low<<", "<<high<<"]"<< std::endl;
    if (low < high) {

//        // Create the partition task
//        co_task<int> partitionTask = partition(low, high);
//        // Start the task
//        partitionTask.resume();
//        // partition is now exhausted for comparisons
//        // and needs answers to go further
//        // Therefore we suspend this coroutine and wait...
//        co_await std::suspend_always();
//
//        // All comparisons have now been resolved and so we
//        // resume partitionTask
//        partitionTask.resume();
//
//        // Partition should now be finished and we retrieve the
//        // pivot index
//
//        int pivot = partitionTask.result();
//        partitionTask.destroyMe();
        int pivot;
        auto partitionTask = new coroTaskVoid(partition(low,high,&pivot));
        co_await scheduler->spawnDependent(partitionTask);
        co_await std::suspend_always();


        // Make the recursive calls by calling spawning them as tasks
        // in the schedular

        if (low < (pivot+1)) {
//            co_task_void recTask1 = quicksort(low, pivot - 1);
            auto recTask1 = new coroTaskVoid(quicksort(low,pivot-1));
            scheduler->spawnIndependent(recTask1);
        }
        if ((pivot+1) < high) {
//            co_task_void recTask2 = quicksort(pivot + 1, high);
            auto recTask2 = new coroTaskVoid(quicksort(pivot+1,high));
            scheduler->spawnIndependent(recTask2);

        }
    }
//    std::cout<< "finishing quicksort coroutine"<<std::endl;
    co_return;
}

template<typename T>
ps_framework::PSQuicksort<T>::~PSQuicksort() {
    // Cleanup
//    res->clear();
//    delete res;
};

#endif