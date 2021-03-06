#include "PSInterfaces.hpp"
#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <vector>
#include <ctime>

#ifndef PS_QUICKSORT
#define PS_QUICKSORT

namespace ps_framework {
    template<typename T>
    class PSQuicksort {
    public:
        PSQuicksort(
                Scheduler *scheduler,
                ComparisonResolver<T> *comparisonResolver,
                std::vector<T> *vec
        );
        ~PSQuicksort();


        void sort();

    protected:
        coroTaskVoid partition(int, int, int*);
        coroTaskVoid randomPartition(int, int, int*);
        coroTaskVoid quicksort(int, int);
        virtual void swap(int, int);

        Scheduler *scheduler;
        ComparisonResolver<T> *comparisonResolver;
        std::vector<T> *arr;
    };
}

// Quicksort inspired by https://www.geeksforgeeks.org/quick-sort/

template <typename T>
ps_framework::PSQuicksort<T>::PSQuicksort(Scheduler *sched, ComparisonResolver<T> *cr, std::vector<T> *vec){
    arr = vec;
    scheduler = sched;
    comparisonResolver = cr;
}

template <typename T>
void ps_framework::PSQuicksort<T>::sort() {
    coroTaskVoid initTask = quicksort(0, arr->size()-1);
    scheduler->spawnIndependent(&initTask);
    scheduler->run();
}

template <typename T>
void ps_framework::PSQuicksort<T>::swap(int i, int j) {
    // Should have mutex for multithreading
    T tmp = (*arr)[i];
    (*arr)[i] = (*arr)[j];
    (*arr)[j] = tmp;
}

template <typename T>
ps_framework::coroTaskVoid ps_framework::PSQuicksort<T>::partition(int low, int high, int *retval){
    // Pivot should be selected randomly
    T pivot = (*arr)[high];

    int i = (low-1);

    for (int j = low; j <= high - 1; j++) {
        // Create new task to handle comparison
        co_await scheduler->spawnDependent(
            new coroTaskVoid(
                [](ComparisonResolver<T> * cv, T elm, T pivot, int *pi, int j, auto qsObj) -> coroTaskVoid {
                    auto res = co_await cv->compare(elm, pivot);
                    if (res == LessThan) {
                        (*pi)++;
                        qsObj->swap((*pi),j);
                    }
                }(comparisonResolver, ((*arr)[j]), pivot, &i, j, this)
            )
        );
    }
    co_await std::suspend_always();

    swap(i+1, high);
    (*retval) = i+1;
    co_return;
};

// Based on https://www.geeksforgeeks.org/quicksort-using-random-pivoting/
template<typename T>
ps_framework::coroTaskVoid ps_framework::PSQuicksort<T>::randomPartition(int low, int high, int *retval) {
    // Use time as seed to random number generator
    std::srand((unsigned) time(0));
    // Generate random number
    int randomNum = low + rand() % (high - low);
    // Swap random with high
    swap(randomNum, high);
    // Create the partition task
    auto partitionTask = new coroTaskVoid(partition(low,high,retval));
    // Spawn partition task
    co_await scheduler->spawnDependent(partitionTask);
    // Wait for partition task to finish
    co_await std::suspend_always();
    // Return
    co_return;
}


template <typename T>
ps_framework::coroTaskVoid ps_framework::PSQuicksort<T>::quicksort(int low, int high){
    if (low < high) {
        int pivot;
        // Create the partition task
        auto partitionTask = new coroTaskVoid(randomPartition(low,high,&pivot));
        // Spawn partition task
        co_await scheduler->spawnDependent(partitionTask);
        // Wait for partition task to finish
        co_await std::suspend_always();


        // Make the recursive calls by spawning them as tasks
        // in the scheduler

        if (low < (pivot+1)) {
            auto recTask1 = new coroTaskVoid(quicksort(low,pivot-1));
            scheduler->spawnIndependent(recTask1);
        }
        if ((pivot+1) < high) {
            auto recTask2 = new coroTaskVoid(quicksort(pivot+1,high));
            scheduler->spawnIndependent(recTask2);
        }
    }
    co_return;
}

template<typename T>
ps_framework::PSQuicksort<T>::~PSQuicksort() {};

#endif