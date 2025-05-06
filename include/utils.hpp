#ifndef UTILS_HPP
#define UTILS_HPP
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

void parse_filename(const std::string &filename, int &rows, int &cols);
void read_array(const std::string &filename, std::vector<int> &array, int expected_rows, int expected_cols);
void find_files(const std::string &folder, std::string &S_file, std::string &T_file,
                int &S_rows, int &S_cols, int &T_rows, int &T_cols);
void read_arrays(const std::string &S_file, const std::string &T_file,
                 int S_rows, int S_cols, int T_rows, int T_cols,
                 std::vector<int> &S, std::vector<int> &T);
std::string get_positions_str(const std::vector<std::pair<int, int>> &positions);
void display_results(const std::string &method, const std::vector<std::pair<int, int>> &best_positions,
                     double best_value, double time);
void write_to_csv(const std::string &filename, int S_rows, int S_cols, int T_rows, int T_cols,
                  const std::string &method, int threads_count, const std::vector<std::pair<int, int>> &best_positions, double best_value, double time);

#endif