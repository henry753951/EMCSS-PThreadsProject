#include "pcc.hpp"
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <pthread.h>
#include <unistd.h>

struct PCCThreadData
{
    const int *X;
    const int *Y;
    size_t start;
    size_t end;
    double mean_X;
    double mean_Y;
    double numerator;
    double sum_sq_X;
    double sum_sq_Y;
};

struct MeanThreadData
{
    const int *data;
    size_t start;
    size_t end;
    double sum;
};

void *mean_thread_func(void *arg)
{
    MeanThreadData *data = (MeanThreadData *)arg;
    data->sum = 0.0;
    for (size_t i = data->start; i < data->end; ++i)
    {
        data->sum += data->data[i];
    }
    return nullptr;
}

void *pcc_thread_func(void *arg)
{
    PCCThreadData *data = (PCCThreadData *)arg;
    double num = 0.0, sx = 0.0, sy = 0.0;
    for (size_t i = data->start; i < data->end; ++i)
    {
        double dx = data->X[i] - data->mean_X;
        double dy = data->Y[i] - data->mean_Y;
        num += dx * dy;
        sx += dx * dx;
        sy += dy * dy;
    }
    data->numerator = num;
    data->sum_sq_X = sx;
    data->sum_sq_Y = sy;
    return nullptr;
}

double compute_pcc(const std::vector<int> &X, const std::vector<int> &Y)
{
    size_t n = X.size();
    if (n != Y.size())
    {
        throw std::invalid_argument("向量大小必須相同");
    }
    double mean_X = std::accumulate(X.begin(), X.end(), 0.0) / n;
    double mean_Y = std::accumulate(Y.begin(), Y.end(), 0.0) / n;
    double numerator = 0.0;
    double sum_sq_X = 0.0;
    double sum_sq_Y = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        double dx = X[i] - mean_X;
        double dy = Y[i] - mean_Y;
        numerator += dx * dy;
        sum_sq_X += dx * dx;
        sum_sq_Y += dy * dy;
    }
    double denom = std::sqrt(sum_sq_X) * std::sqrt(sum_sq_Y);
    if (denom == 0.0)
    {
        return 0.0; // 處理常數向量
    }
    return numerator / denom;
}

double compute_pcc_parallel(const std::vector<int> &X, const std::vector<int> &Y)
{
    size_t n = X.size();
    if (n != Y.size())
    {
        throw std::invalid_argument("向量大小必須相同");
    }
    if (n == 0)
        return 0.0;

    // 動態決定執行緒數：最多使用系統核心數，最少1個
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    size_t num_threads = std::min(static_cast<size_t>(num_cores), n / 16 + 1);
    num_threads = std::max(size_t(1), num_threads);

    // 平行計算均值
    std::vector<MeanThreadData> mean_data_x(num_threads);
    std::vector<MeanThreadData> mean_data_y(num_threads);
    std::vector<pthread_t> threads(num_threads);
    size_t chunk_size = n / num_threads;

    // 初始化均值計算執行緒資料
    for (size_t i = 0; i < num_threads; ++i)
    {
        mean_data_x[i].data = X.data();
        mean_data_y[i].data = Y.data();
        mean_data_x[i].start = mean_data_y[i].start = i * chunk_size;
        mean_data_x[i].end = mean_data_y[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
    }

    // 創建執行緒計算均值
    for (size_t i = 0; i < num_threads; ++i)
    {
        if (pthread_create(&threads[i], nullptr, mean_thread_func, &mean_data_x[i]) != 0 ||
            pthread_create(&threads[i], nullptr, mean_thread_func, &mean_data_y[i]) != 0)
        {
            throw std::runtime_error("無法創建執行緒");
        }
    }

    // 收集均值結果
    double sum_X = 0.0, sum_Y = 0.0;
    for (size_t i = 0; i < num_threads; ++i)
    {
        pthread_join(threads[i], nullptr);
        sum_X += mean_data_x[i].sum;
        sum_Y += mean_data_y[i].sum;
    }
    double mean_X = sum_X / n;
    double mean_Y = sum_Y / n;

    // 平行計算 PCC
    std::vector<PCCThreadData> thread_data(num_threads);
    for (size_t i = 0; i < num_threads; ++i)
    {
        thread_data[i].X = X.data();
        thread_data[i].Y = Y.data();
        thread_data[i].mean_X = mean_X;
        thread_data[i].mean_Y = mean_Y;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
    }

    // 創建執行緒計算 PCC
    for (size_t i = 0; i < num_threads; ++i)
    {
        if (pthread_create(&threads[i], nullptr, pcc_thread_func, &thread_data[i]) != 0)
        {
            throw std::runtime_error("無法創建執行緒");
        }
    }

    // 收集結果
    double numerator = 0.0, sum_sq_X = 0.0, sum_sq_Y = 0.0;
    for (size_t i = 0; i < num_threads; ++i)
    {
        pthread_join(threads[i], nullptr);
        numerator += thread_data[i].numerator;
        sum_sq_X += thread_data[i].sum_sq_X;
        sum_sq_Y += thread_data[i].sum_sq_Y;
    }

    double denom = std::sqrt(sum_sq_X) * std::sqrt(sum_sq_Y);
    if (denom == 0.0)
    {
        return 0.0;
    }
    return numerator / denom;
}