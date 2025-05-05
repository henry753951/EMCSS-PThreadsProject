#ifndef COMPUTE_HPP
#define COMPUTE_HPP
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <limits>
#include <mutex>
#include <vector>
#include <functional>
#include <utility>
#include <mutex>

void compute(const std::vector<int> &S, const std::vector<int> &T,
             int S_rows, int S_cols, int T_rows, int T_cols,
             std::function<double(const std::vector<int> &, const std::vector<int> &)> compute_func,
             bool find_max, std::vector<std::pair<int, int>> &best_positions, double &best_value);

struct ComputeThreadData
{
    const std::vector<int> *S;
    const std::vector<int> *T;
    int S_rows, S_cols, T_rows, T_cols;
    std::function<double(const std::vector<int> &, const std::vector<int> &)> compute_func;
    bool find_max;
    int start_i, end_i;
    double local_best_value;
    std::vector<std::pair<int, int>> local_best_positions;
};

void compute_parallel(const std::vector<int> &S, const std::vector<int> &T,
                      int S_rows, int S_cols, int T_rows, int T_cols,
                      std::function<double(const std::vector<int> &, const std::vector<int> &)> compute_func,
                      bool find_max, std::vector<std::pair<int, int>> &best_positions, double &best_value);

#endif