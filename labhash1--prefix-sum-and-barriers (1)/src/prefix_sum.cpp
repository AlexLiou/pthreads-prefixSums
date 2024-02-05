#include "prefix_sum.h"
#include "helpers.h"
#include "spin_barrier.h"
#include <mutex>

struct ThreadArgs {
    prefix_sum_args_t* ps_args;
    prefix_sum* instance;
};

using namespace std;
prefix_sum::prefix_sum(pthread_t *threads, int num_threads, std::vector<int>& input)
    : num_threads(num_threads), input(input), threads(threads) {
    total_elements = input.size();
    // threads.resize(num_threads);


    pthread_barrier_init(&barrier, nullptr, num_threads);
    spinBarrier = new spin_barrier();
}

prefix_sum::~prefix_sum() {
    pthread_barrier_destroy(&barrier);
    delete spinBarrier;
}

void* prefix_sum::worker(void* args) {
    
    ThreadArgs* threadArgs = static_cast<ThreadArgs*>(args);
    prefix_sum* prefixSum = threadArgs->instance;
    prefix_sum_args_t* ps_args = threadArgs->ps_args;

    const int num_threads = ps_args->n_threads;
    const int thread_id = ps_args->t_id;
    const int n_vals = ps_args->n_vals;

    // Calculate the range of elements for this thread_id
    const int elements_per_thread = n_vals / num_threads;
    const int start = thread_id * elements_per_thread;
    const int end = (thread_id == num_threads - 1) ? n_vals : start + elements_per_thread;

    // Lock the mutex
    if (ps_args->spin) {
        prefixSum->spinBarrier->lock();
    } else {
        pthread_mutex_lock(&prefixSum->mutex);
    }

    // Calculate the local prefix sum for the assigned elements
    int sum = 0;
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < n_vals) {
            sum += ps_args->input_vals[i];
            ps_args->output_vals[i] = sum;
        } else {
            std::cerr << "Error Out of Bounds" << std::endl;
            exit(1);
        }
    }
    
    // Ensure all threads have completed their local prefix scan calculations.
    if (ps_args->spin) {
        prefixSum->spinBarrier->unlock();
    } else {
        pthread_mutex_unlock(&prefixSum->mutex);
        pthread_barrier_wait(&prefixSum->barrier);
    }
    

    // Perform the final update in thread order 
    for (int t = 0; t < num_threads; ++t) {
        if (ps_args->spin) {
            prefixSum->spinBarrier->lock();
        }
        if (thread_id == t) {
            // Calculate the thread's contribution to the final prefix sum
            int thread_contribution = 0;
            if (thread_id > 0) {
                thread_contribution = ps_args->output_vals[start - 1];
            }
            // Update the local partial sums with the final prefix sum
            for (int i = start; i < end; ++i) {
                ps_args->output_vals[i] += thread_contribution;
            }
        }
        // Synchronize again before proceeding
        if (ps_args->spin) {
            prefixSum->spinBarrier->unlock();
        } else {
            pthread_barrier_wait(&prefixSum->barrier);
        }
    }

    return nullptr;
}

void prefix_sum::compute(struct prefix_sum_args_t *args) {
    ThreadArgs* threadArgsArray = new ThreadArgs[num_threads];
    for (int i = 0; i < num_threads; ++i) {
        threadArgsArray[i].ps_args = &(args[i]);
        threadArgsArray[i].instance = this;
        pthread_create(&(threads[i]), nullptr, worker, (void*)&(threadArgsArray[i]));
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    delete[] threadArgsArray;
}