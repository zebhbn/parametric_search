//
// Created by zeb on 08/06/2022.
//
#include "PSQuicksort.hpp"
#include <mutex>
#include <condition_variable>

#ifndef MINIMUMMEANCYCLE_MULTITQUICKSORT_HPP
#define MINIMUMMEANCYCLE_MULTITQUICKSORT_HPP

template <typename T>
class MultiTQuicksort : public ps_framework::PSQuicksort<T> {
    std::mutex swapMutex;
    std::condition_variable swapCV;
    using ps_framework::PSQuicksort<T>::arr;
    void swap(int i, int j) {
        // Should have mutex for multithreading
        T tmp = (*arr)[i];
        (*arr)[i] = (*arr)[j];
        (*arr)[j] = tmp;
    }
};

#endif //MINIMUMMEANCYCLE_MULTITQUICKSORT_HPP
