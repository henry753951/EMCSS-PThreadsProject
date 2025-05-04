#ifndef SSD_HPP
#define SSD_HPP

#include <vector>

double compute_ssd(const std::vector<int> &X, const std::vector<int> &Y);
double compute_ssd_parallel(const std::vector<int> &X, const std::vector<int> &Y);

#endif