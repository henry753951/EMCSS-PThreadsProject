#include "utils.hpp"

void parse_filename(const std::string &filename, int &rows, int &cols)
{
    std::regex pattern(R"(_(\d+)_(\d+)\.txt$)");
    std::smatch match;
    if (std::regex_search(filename, match, pattern))
    {
        rows = std::stoi(match[1]);
        cols = std::stoi(match[2]);
    }
    else
    {
        throw std::runtime_error("Invalid filename format: " + filename);
    }
}

void read_array(const std::string &filename, std::vector<int> &array, int expected_rows, int expected_cols)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::string line;
    int rows = 0;
    int cols = -1;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::vector<int> row;
        std::string num;
        while (std::getline(iss, num, ','))
        {
            try
            {
                int val = std::stoi(num);
                if (val < 0 || val > 9)
                {
                    throw std::runtime_error("Values must be between 0 and 9 in file: " + filename);
                }
                row.push_back(val);
            }
            catch (const std::invalid_argument &)
            {
                throw std::runtime_error("Invalid number format in file: " + filename);
            }
        }
        if (cols == -1)
        {
            cols = row.size();
        }
        else if (row.size() != static_cast<size_t>(cols))
        {
            throw std::runtime_error("Inconsistent number of columns in file: " + filename);
        }
        for (int val : row)
        {
            array.push_back(val);
        }
        rows++;
    }
    std::cout << "Read " << rows << " rows and " << cols << " columns from file: " << filename << std::endl;
    if (rows != expected_rows || cols != expected_cols)
    {
        throw std::runtime_error("Array dimensions do not match filename: " + filename);
    }
}

void find_files(const std::string &folder_path, std::string &S_file, std::string &T_file,
                int &S_rows, int &S_cols, int &T_rows, int &T_cols)
{
    std::string folder_name = fs::path(folder_path).filename().string();
    for (const auto &entry : fs::directory_iterator(folder_path))
    {
        std::string filename = entry.path().filename().string();
        if (filename.find("S" + folder_name + "_") == 0)
        {
            S_file = entry.path().string();
            parse_filename(filename, S_rows, S_cols);
        }
        else if (filename.find("T" + folder_name + "_") == 0)
        {
            T_file = entry.path().string();
            parse_filename(filename, T_rows, T_cols);
        }
    }

    if (S_file.empty() || T_file.empty())
    {
        throw std::runtime_error("Could not find S or T file in folder: " + folder_path);
    }
}

void read_arrays(const std::string &S_file, const std::string &T_file,
                 int S_rows, int S_cols, int T_rows, int T_cols,
                 std::vector<int> &S, std::vector<int> &T)
{
    read_array(S_file, S, S_rows, S_cols);
    read_array(T_file, T, T_rows, T_cols);
}

void print_positions(const std::vector<std::pair<int, int>> &positions)
{
    for (const auto &pos : positions)
    {
        std::cout << "(" << pos.first + 1 << "," << pos.second + 1 << ")" << std::endl;
    }
}