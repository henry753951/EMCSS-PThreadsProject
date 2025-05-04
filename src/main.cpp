#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "compute.hpp"
#include "pcc.hpp"
#include "ssd.hpp"
#include "utils.hpp"

struct Method
{
    std::string name;
    std::function<double(const std::vector<int> &, const std::vector<int> &)>
        compute_func;
    bool find_max;
};

std::string get_folder_path()
{
    std::string folder;
    std::cout << "Path: ";
    std::cin >> folder;
    if (!folder.empty() && folder[0] == '/')
    {
        folder = folder.substr(1);
    }
    return folder;
}

bool use_parallel_computation()
{
    std::cout << "Use parallel? (y/n): ";
    char choice;
    std::cin >> choice;
    return (choice == 'y' || choice == 'Y');
}

double run_method(const Method &method, const std::vector<int> &S,
                  const std::vector<int> &T, int S_rows, int S_cols, int T_rows,
                  int T_cols, bool use_parallel)
{
    std::cout << "\n[Computing " << method.name << "...]\n";

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::pair<int, int>> best_positions;
    double best_value;
    if (use_parallel)
    {
        compute_parallel(S, T, S_rows, S_cols, T_rows, T_cols, method.compute_func,
                         method.find_max, best_positions, best_value);
    }
    else
    {
        compute(S, T, S_rows, S_cols, T_rows, T_cols, method.compute_func,
                method.find_max, best_positions, best_value);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double time =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() /
        1e6;

    std::cout << "Best " << method.name << " positions (value: " << std::fixed
              << std::setprecision(2) << best_value << "):\n";
    print_positions(best_positions);
    std::cout << method.name << " computation time: " << std::fixed
              << std::setprecision(6) << time << " seconds\n";

    return time;
}

int main()
{
    std::string folder = get_folder_path();
    bool use_parallel = use_parallel_computation();

    try
    {
        std::string S_file, T_file;
        int S_rows, S_cols, T_rows, T_cols;
        find_files(folder, S_file, T_file, S_rows, S_cols, T_rows, T_cols);

        std::vector<int> S, T;
        read_arrays(S_file, T_file, S_rows, S_cols, T_rows, T_cols, S, T);

        std::vector<Method> methods = {
            {"PCC", compute_pcc, true},
            {"SSD", compute_ssd, false},
        };

        for (const auto &method : methods)
        {
            run_method(method, S, T, S_rows, S_cols, T_rows, T_cols, use_parallel);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}