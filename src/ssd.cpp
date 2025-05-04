#include "ssd.hpp"
#include <stdexcept>

double compute_ssd(const std::vector<int> &X, const std::vector<int> &Y)
{
    size_t n = X.size();
    if (n != Y.size())
    {
        throw std::invalid_argument("Vector sizes must be the same");
    }
    double ssd = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        double diff = X[i] - Y[i];
        ssd += diff * diff;
    }
    return ssd;
}
