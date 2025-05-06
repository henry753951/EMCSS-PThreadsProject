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

std::string get_positions_str(const std::vector<std::pair<int, int>> &positions)
{
    std::string ss;
    for (const auto &pos : positions)
    {
        ss += "(" + std::to_string(pos.first) + "," + std::to_string(pos.second) + ") ";
    }
    return ss;
}

void display_results(const std::string &method, const std::vector<std::pair<int, int>> &best_positions,
                     double best_value, double time)
{
    std::cout << "----------------------------------------\n";
    std::cout << "Results for " << method << ":\n";
    std::cout << "Best value: " << std::fixed << std::setprecision(2) << best_value << "\n";
    std::cout << "Best positions:\n";
    if (best_positions.empty())
    {
        std::cout << "No positions found.\n";
    }
    else
    {
        std::cout << get_positions_str(best_positions) << "\n";
    }
    std::cout << "Computation time: " << std::fixed << std::setprecision(6)
              << time << " seconds\n";
    std::cout << "----------------------------------------\n";
}

void write_to_csv(const std::string &data_path, int s_rows, int s_cols, int t_rows, int t_cols,
                  const std::string &method, int threads_count, const std::vector<std::pair<int, int>> &best_positions,
                  double best_value, double time)
{
    std::string csv_path = "outputs/";
    std::string filename = data_path;
    std::replace(filename.begin(), filename.end(), '/', '_');
    filename += "_" + std::to_string(s_rows) + "x" + std::to_string(s_cols) + "_" +
                std::to_string(t_rows) + "x" + std::to_string(t_cols) + ".csv";
    csv_path += filename;

    if (!fs::exists("outputs"))
    {
        if (!fs::create_directory("outputs"))
        {
            throw std::runtime_error("Failed to create outputs directory");
        }
    }

    std::ofstream csv_file(csv_path, std::ios::app);
    if (!csv_file.is_open())
    {
        throw std::runtime_error("Failed to open CSV file: " + csv_path);
    }

    if (csv_file.tellp() == 0)
    {
        csv_file << "methods,threads,best_positions,best_value,cost_time\n";
    }

    std::string positions_str = get_positions_str(best_positions);
    // Quote positions string to handle spaces and commas
    positions_str = "\"" + positions_str + "\"";

    csv_file << std::fixed << std::setprecision(2);
    csv_file << method << ","
             << threads_count << ","
             << positions_str << ","
             << best_value << ","
             << std::setprecision(6) << time << "\n";

    csv_file.close();
}