#ifndef PCC_HPP
#define PCC_HPP

#include <vector>

double compute_pcc(const std::vector<int> &X, const std::vector<int> &Y);
double compute_pcc_parallel(const std::vector<int> &X, const std::vector<int> &Y);

#endif