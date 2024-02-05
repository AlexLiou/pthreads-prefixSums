#include <iostream>
#include <argparse.h>
#include <threads.h>
#include <io.h>
#include <chrono>
#include <cstring>
#include "operators.h"
#include "helpers.h"
#include "prefix_sum.h"
#include "spin_barrier.h"
#include <mutex>

using namespace std;

int main(int argc, char **argv)
{
    // Parse args
    struct options_t opts;
    get_opts(argc, argv, &opts);


    bool sequential = false;
    if (opts.n_threads == 0) {
        opts.n_threads = 1;
        sequential = true;
    }

    // Setup threads
    pthread_t *threads = sequential ? NULL : alloc_threads(opts.n_threads);;

    // Setup args & read input data
    prefix_sum_args_t *ps_args = alloc_args(opts.n_threads);
    int n_vals;
    int *input_vals, *output_vals;
    read_file(&opts, &n_vals, &input_vals, &output_vals);

    // Calculate the next power of 2 for input size
    int padded_size = next_power_of_two(n_vals);

    // New array for padded input values
    int *padded_input_vals = new int[padded_size];

    // initialize the entire array to zero
    memset(padded_input_vals, 0, padded_size * sizeof(int));

    // Copy the original input into the padded array
    memcpy(padded_input_vals, input_vals, n_vals * sizeof(int));

    //"op" is the operator you have to use, but you can use "add" to test
    int (*scan_operator)(int, int, int);
    scan_operator = op;
    // scan_operator = add;

    fill_args(ps_args, opts.n_threads, padded_size, padded_input_vals, output_vals,
        opts.spin, scan_operator, opts.n_loops);

    // Prefix Sum Instance
    std::vector<int> input_vec(padded_input_vals, padded_input_vals + padded_size);
    prefix_sum prefix_sum(threads, opts.n_threads, input_vec);

    // Start timer
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Start Timer" << endl;
    if (sequential)  {
        //sequential prefix scan
        output_vals[0] = input_vals[0];
        for (int i = 1; i < n_vals; ++i) {
            //y_i = y_{i-1}  <op>  x_i
            output_vals[i] = scan_operator(output_vals[i-1], input_vals[i], ps_args->n_loops);
        }
    }
    else {
        prefix_sum.compute(ps_args);

    }

    //End timer and print out elapsed
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time: " << diff.count() << std::endl;

    // Write output data
    write_file(&opts, &(ps_args[0]));

    // Free other buffers
    free(threads);
    free(ps_args);
}
