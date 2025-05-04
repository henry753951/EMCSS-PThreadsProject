#include "ssd.hpp"
#include <cstddef>
#include <stdexcept>
#include <pthread.h>
#include <unistd.h>

struct SSDThreadData
{
    const int *X;
    const int *Y;
    size_t start;
    size_t end;
    double ssd;
};

void *ssd_thread_func(void *arg)
{
    SSDThreadData *data = (SSDThreadData *)arg;
    double sum = 0.0;
    for (size_t i = data->start; i < data->end; ++i)
    {
        double diff = data->X[i] - data->Y[i];
        sum += diff * diff;
    }
    data->ssd = sum;
    return nullptr;
}

double compute_ssd(const std::vector<int> &X, const std::vector<int> &Y)
{
    size_t n = X.size();
    if (n != Y.size())
    {
        throw std::invalid_argument("向量大小必須相同");
    }
    double ssd = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        double diff = X[i] - Y[i];
        ssd += diff * diff;
    }
    return ssd;
}

double compute_ssd_parallel(const std::vector<int> &X, const std::vector<int> &Y)
{
    size_t n = X.size();
    if (n != Y.size())
    {
        throw std::invalid_argument("向量大小必須相同");
    }
    if (n == 0)
        return 0.0;

    // 動態決定執行緒數
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    size_t num_threads = std::min(static_cast<size_t>(num_cores), n / 16 + 1);
    num_threads = std::max(size_t(1), num_threads);

    std::vector<pthread_t> threads(num_threads);
    std::vector<SSDThreadData> thread_data(num_threads);
    size_t chunk_size = n / num_threads;

    // 初始化執行緒資料
    for (size_t i = 0; i < num_threads; ++i)
    {
        thread_data[i].X = X.data();
        thread_data[i].Y = Y.data();
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
    }

    // 創建執行緒
    for (size_t i = 0; i < num_threads; ++i)
    {
        if (pthread_create(&threads[i], nullptr, ssd_thread_func, &thread_data[i]) != 0)
        {
            throw std::runtime_error("無法創建執行緒");
        }
    }
    // 收集結果
    double ssd = 0.0;
    for (size_t i = 0; i < num_threads; ++i)
    {
        pthread_join(threads[i], nullptr);
        ssd += thread_data[i].ssd;
    }

    return ssd;
}