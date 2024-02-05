#pragma once

#include <stdlib.h>
#include <pthread.h>
#include "helpers.h"
#include "spin_barrier.h"
#include <iostream>
#include <vector>
#include <mutex>

class prefix_sum {
    public:
        prefix_sum(pthread_t *threads, int num_threads, std::vector<int>& input);
        ~prefix_sum();
        void compute(struct prefix_sum_args_t *args);
        pthread_barrier_t barrier;
        spin_barrier* spinBarrier;

    private:
        int num_threads;
        std::vector<int>& input;
        int total_elements;
        
        pthread_t* threads;

        static void* worker(void* args);
        std::mutex barrier_mutex;
        pthread_mutex_t mutex;
};