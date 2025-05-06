#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "compute.hpp"
#include "pcc.hpp"
#include "ssd.hpp"
#include "utils.hpp"

struct Method
{
    std::string name;
    std::function<double(const std::vector<int> &, const std::vector<int> &)> compute_func;
    bool find_max;
};

std::string get_folder_path()
{
    std::string folder;
    std::cout << "Enter folder path: ";
    std::cin >> folder;
    if (!folder.empty() && folder[0] == '/')
    {
        folder = folder.substr(1);
    }
    return folder;
}

void display_system_info(int num_cores, int max_threads, int s_rows, int s_cols, int t_rows, int t_cols)
{
    std::cout << "\nSystem Information:\n";
    std::cout << "-------------------\n";
    std::cout << "Available cores: " << num_cores << "\n";
    std::cout << "\nComputation Parameters:\n";
    std::cout << "-------------------\n";
    std::cout << "Maximum threads: " << max_threads << "\n";
    std::cout << "Matrix S dimensions: " << s_rows << "x" << s_cols << "\n";
    std::cout << "Matrix T dimensions: " << t_rows << "x" << t_cols << "\n\n";
}

double run_method(const Method &method, const std::vector<int> &S, const std::vector<int> &T,
                  int s_rows, int s_cols, int t_rows, int t_cols, int threads_count,
                  const std::string &data_path)
{
    std::cout << "\n[Computing " << method.name << " with " << threads_count
              << " thread" << (threads_count > 1 ? "s" : "") << "]\n";

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::pair<int, int>> best_positions;
    double best_value;
    if (threads_count > 1)
    {
        compute_parallel(S, T, s_rows, s_cols, t_rows, t_cols, method.compute_func,
                         method.find_max, best_positions, best_value, threads_count);
    }
    else
    {
        compute(S, T, s_rows, s_cols, t_rows, t_cols, method.compute_func,
                method.find_max, best_positions, best_value);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1e6;

    display_results(method.name, best_positions, best_value, time);
    write_to_csv(data_path, s_rows, s_cols, t_rows, t_cols, method.name, threads_count,
                 best_positions, best_value, time);

    return time;
}

int main()
{
    std::cout << std::fixed << std::setprecision(6);
    std::string folder = get_folder_path();
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    try
    {
        std::string s_file, t_file;
        int s_rows, s_cols, t_rows, t_cols;
        find_files(folder, s_file, t_file, s_rows, s_cols, t_rows, t_cols);

        std::vector<int> S, T;
        read_arrays(s_file, t_file, s_rows, s_cols, t_rows, t_cols, S, T);

        std::vector<Method> methods = {
            {"PCC", compute_pcc, true},
            {"SSD", compute_ssd, false}};

        int max_i = t_rows - s_rows + 1;
        int max_threads = std::min(num_cores, max_i);

        display_system_info(num_cores, max_threads, s_rows, s_cols, t_rows, t_cols);

        for (int threads_count = 1; threads_count <= max_threads; ++threads_count)
        {
            std::cout << "=== Starting computations with " << threads_count
                      << " thread" << (threads_count > 1 ? "s" : "") << " ===\n";

            for (const auto &method : methods)
            {
                run_method(method, S, T, s_rows, s_cols, t_rows, t_cols, threads_count, folder);
            }
            std::cout << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}