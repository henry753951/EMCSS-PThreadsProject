
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <filesystem>
#include <regex>
#include <limits>
#include <functional>
#include "utils.hpp"
#include "pcc.hpp"
#include "ssd.hpp"
using namespace std;

void compute(const vector<int> &S, const vector<int> &T, int S_rows, int S_cols, int T_rows, int T_cols,
             function<double(const vector<int> &, const vector<int> &)> compute_func,
             bool find_max, vector<pair<int, int>> &best_positions, double &best_value)
{
    const double epsilon = 1e-10; // For floating-point comparison
    best_value = find_max ? -numeric_limits<double>::max() : numeric_limits<double>::max();

    for (int i = 0; i <= T_rows - S_rows; ++i)
    {
        for (int j = 0; j <= T_cols - S_cols; ++j)
        {
            // Extract sub-array from T
            vector<int> sub_T;
            for (int k = 0; k < S_rows; ++k)
            {
                for (int l = 0; l < S_cols; ++l)
                {
                    sub_T.push_back(T[(i + k) * T_cols + (j + l)]);
                }
            }
            // Compute similarity
            double value = compute_func(sub_T, S);
            // Update best positions
            if (find_max)
            {
                if (value > best_value + epsilon)
                {
                    best_value = value;
                    best_positions.clear();
                    best_positions.push_back({i, j});
                }
                else if (abs(value - best_value) < epsilon)
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
                else if (abs(value - best_value) < epsilon)
                {
                    best_positions.push_back({i, j});
                }
            }
        }
    }
}

struct Method
{
    string name;
    function<double(const vector<int> &, const vector<int> &)> compute_func;
    bool find_max;
};

int main()
{
    // Read folder name
    string folder;
    cin >> folder;
    if (folder[0] == '/')
    {
        folder = folder.substr(1);
    }

    try
    {
        // Find S and T files
        string S_file, T_file;
        int S_rows, S_cols, T_rows, T_cols;
        find_files(folder, S_file, T_file, S_rows, S_cols, T_rows, T_cols);

        // Read arrays
        vector<int> S, T;
        read_arrays(S_file, T_file, S_rows, S_cols, T_rows, T_cols, S, T);

        vector<Method> methods = {
            {"PCC", compute_pcc, true},
            {"SSD", compute_ssd, false},
            {"PCC Parallel", compute_pcc_parallel, true},
            {"SSD Parallel", compute_ssd_parallel, false},
        };

        for (const auto &method : methods)
        {
            cout << endl
                 << "[Computing " << method.name << "...]" << endl;
            vector<pair<int, int>> best_positions;
            double best_value;
            auto start_time = chrono::high_resolution_clock::now();
            compute(S, T, S_rows, S_cols, T_rows, T_cols, method.compute_func, method.name == "PCC", best_positions, best_value);
            auto end_time = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
            double computation_time = duration.count() / 1e6; // Units : seconds

            // Output results
            cout << "Best " << method.name << " positions (value: " << fixed << setprecision(2) << best_value << "):" << endl;
            print_positions(best_positions);
            cout << method.name << " computation time: " << fixed << setprecision(6) << computation_time << " seconds" << endl;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}