#include "pcc.hpp"
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <iostream>

double compute_pcc(const std::vector<int> &X, const std::vector<int> &Y)
{
    size_t n = X.size();
    if (n != Y.size())
    {
        throw std::invalid_argument("Vector sizes must be the same");
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
        return 0.0;
    }
    return numerator / denom;
}
