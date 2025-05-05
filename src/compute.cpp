#include "compute.hpp"

// Without parallelization
void compute(const std::vector<int> &S, const std::vector<int> &T,
             int S_rows, int S_cols, int T_rows, int T_cols,
             std::function<double(const std::vector<int> &, const std::vector<int> &)> compute_func,
             bool find_max, std::vector<std::pair<int, int>> &best_positions, double &best_value)
{
    const double epsilon = 1e-10;
    best_value = find_max ? -std::numeric_limits<double>::max() : std::numeric_limits<double>::max();
    best_positions.clear();

    for (int i = 0; i <= T_rows - S_rows; ++i)
    {
        for (int j = 0; j <= T_cols - S_cols; ++j)
        {
            std::vector<int> sub_T;
            sub_T.reserve(S_rows * S_cols);
            for (int k = 0; k < S_rows; ++k)
            {
                for (int l = 0; l < S_cols; ++l)
                {
                    sub_T.push_back(T[(i + k) * T_cols + (j + l)]);
                }
            }
            double value = compute_func(sub_T, S);
            if (find_max)
            {
                if (value > best_value + epsilon)
                {
                    best_value = value;
                    best_positions.clear();
                    best_positions.push_back({i, j});
                }
                else if (std::abs(value - best_value) < epsilon)
                {
                    best_positions.push_back({i, j});
                }
            }
            else
            {
                if (value < best_value - epsilon)
                {
                    best_value = value;
                    best_positions.clear();
                    best_positions.push_back({i, j});
                }
                else if (std::abs(value - best_value) < epsilon)
                {
                    best_positions.push_back({i, j});
                }
            }
        }
    }
}

// Thread function for parallel computation
void *compute_thread_func(void *arg)
{
    ComputeThreadData *data = (ComputeThreadData *)arg;
    const double epsilon = 1e-10;
    data->local_best_value = data->find_max ? -std::numeric_limits<double>::max() : std::numeric_limits<double>::max();
    data->local_best_positions.clear();

    for (int i = data->start_i; i < data->end_i; ++i)
    {
        for (int j = 0; j <= data->T_cols - data->S_cols; ++j)
        {
            std::vector<int> sub_T;
            sub_T.reserve(data->S_rows * data->S_cols);
            for (int k = 0; k < data->S_rows; ++k)
            {
                for (int l = 0; l < data->S_cols; ++l)
                {
                    sub_T.push_back((*data->T)[(i + k) * data->T_cols + (j + l)]);
                }
            }
            double value = data->compute_func(sub_T, *data->S);

            // Lock the mutex to update the local best value and positions
            if (data->find_max)
            {
                if (value > data->local_best_value + epsilon)
                {
                    data->local_best_value = value;
                    data->local_best_positions.clear();
                    data->local_best_positions.push_back({i, j});
                }
                else if (std::abs(value - data->local_best_value) < epsilon)
                {
                    data->local_best_positions.push_back({i, j});
                }
            }
            else
            {
                if (value < data->local_best_value - epsilon)
                {
                    data->local_best_value = value;
                    data->local_best_positions.clear();
                    data->local_best_positions.push_back({i, j});
                }
                else if (std::abs(value - data->local_best_value) < epsilon)
                {
                    data->local_best_positions.push_back({i, j});
                }
            }
        }
    }
    return nullptr;
}

// With parallelization
void compute_parallel(const std::vector<int> &S, const std::vector<int> &T,
                      int S_rows, int S_cols, int T_rows, int T_cols,
                      std::function<double(const std::vector<int> &, const std::vector<int> &)> compute_func,
                      bool find_max, std::vector<std::pair<int, int>> &best_positions, double &best_value)
{
    const double epsilon = 1e-10;
    best_value = find_max ? -std::numeric_limits<double>::max() : std::numeric_limits<double>::max();
    best_positions.clear();

    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores <= 0)
        num_cores = 1;
    int max_i = T_rows - S_rows + 1;
    size_t num_threads = std::min(static_cast<size_t>(num_cores), static_cast<size_t>(max_i));
    num_threads = std::max(size_t(1), num_threads);

    std::vector<pthread_t> threads(num_threads);
    std::vector<ComputeThreadData> thread_data(num_threads);
    std::mutex mtx;

    int chunk_size = max_i / num_threads;
    for (size_t t = 0; t < num_threads; ++t)
    {
        thread_data[t].S = &S;
        thread_data[t].T = &T;
        thread_data[t].S_rows = S_rows;
        thread_data[t].S_cols = S_cols;
        thread_data[t].T_rows = T_rows;
        thread_data[t].T_cols = T_cols;
        thread_data[t].compute_func = compute_func;
        thread_data[t].find_max = find_max;
        thread_data[t].start_i = t * chunk_size;
        thread_data[t].end_i = (t == num_threads - 1) ? max_i : (t + 1) * chunk_size;
        thread_data[t].mtx = &mtx;
    }

    size_t successful_threads = 0;
    for (size_t t = 0; t < num_threads; ++t)
    {
        if (pthread_create(&threads[t], nullptr, compute_thread_func, &thread_data[t]) != 0)
        {
            std::cerr << "無法創建執行緒 " << t << std::endl;
            break;
        }
        successful_threads++;
    }

    if (successful_threads == 0)
    {
        std::cerr << "所有執行緒創建失敗，使用單執行緒模式" << std::endl;
        compute(S, T, S_rows, S_cols, T_rows, T_cols, compute_func, find_max, best_positions, best_value);
        return;
    }

    for (size_t t = 0; t < successful_threads; ++t)
    {
        pthread_join(threads[t], nullptr);
    }

    for (size_t t = 0; t < successful_threads; ++t)
    {
        if (find_max)
        {
            if (thread_data[t].local_best_value > best_value + epsilon)
            {
                best_value = thread_data[t].local_best_value;
                best_positions = thread_data[t].local_best_positions;
            }
            else if (std::abs(thread_data[t].local_best_value - best_value) < epsilon)
            {
                best_positions.insert(best_positions.end(),
                                      thread_data[t].local_best_positions.begin(),
                                      thread_data[t].local_best_positions.end());
            }
        }
        else
        {
            if (thread_data[t].local_best_value < best_value - epsilon)
            {
                best_value = thread_data[t].local_best_value;
                best_positions = thread_data[t].local_best_positions;
            }
            else if (std::abs(thread_data[t].local_best_value - best_value) < epsilon)
            {
                best_positions.insert(best_positions.end(),
                                      thread_data[t].local_best_positions.begin(),
                                      thread_data[t].local_best_positions.end());
            }
        }
    }
}