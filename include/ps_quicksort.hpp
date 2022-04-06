#include "ps_framework.hpp"
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

        // For testing purposes
//        PSQuicksort();

        void sort();

    private:
        co_task<int> partition(int, int);

        co_task<void> quicksort(int, int);

        void swap(int, int);

//        PSFramework<T> *psframe;
        ISchedular<T> *schedular;
        std::vector<T> *arr;
        std::vector<cmp_res> *res;
    };
}

#endif