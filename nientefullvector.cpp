#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#define CSV_LINE_LENGTH 61

int skip_lines(std::ifstream& file, unsigned long long num) {
    std::string line;
    for (unsigned long long run = 0; run < num; ++run) {
        if (!std::getline(file, line)) {
            return -1;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    std::ifstream amatrix_file("a.mat");
    std::string amatrix_line;
    unsigned long long line_count = 0;
    const char* end_ptr;
    long amatrix_width = 0;
    long amatrix_height = 0;
    long tot = 0;

    while (std::getline(amatrix_file, amatrix_line)) {
        if (line_count > 1) {
            break;
        } else if (line_count == 0) {
            errno = 0;
            amatrix_height = std::strtol(amatrix_line.c_str(), const_cast<char**>(&end_ptr), 10);
            std::cout << "height: " << amatrix_height << std::endl;
        } else if (line_count == 1) {
            errno = 0;
            amatrix_width = std::strtol(amatrix_line.c_str(), const_cast<char**>(&end_ptr), 10);
            std::cout << "width: " << amatrix_width << std::endl;
        }
        ++line_count;
    }
    std::cout << "line count " << line_count << std::endl;

    uint64_t amatrix_max_width = 0;
    uint64_t amatrix_current_row = 0;
    uint64_t amatrix_count_used = 0;

    std::vector<std::vector<double>> amatrix_values(amatrix_height, std::vector<double>(amatrix_width, 0));
    std::vector<std::vector<uint64_t>> amatrix_indices(amatrix_height, std::vector<uint64_t>(amatrix_width, 0));

    while (std::getline(amatrix_file, amatrix_line)) {
        std::istringstream token_stream(amatrix_line);
        std::string token;

        std::getline(token_stream, token, ' ');
        uint64_t amatrix_row = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));
        std::getline(token_stream, token, ' ');
        uint64_t amatrix_column = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));
        if (amatrix_row > amatrix_current_row) {
            amatrix_current_row = amatrix_row;
            if (amatrix_max_width < amatrix_count_used) {
                amatrix_max_width = amatrix_count_used;
            }
            amatrix_count_used = 0;
        }

        ++amatrix_count_used;
    }

    if (amatrix_max_width <= 0) {
        amatrix_max_width = static_cast<uint64_t>(amatrix_width);
    }

    printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", amatrix_width, amatrix_max_width);

    // Initialize vectors
    std::vector<double> amatrix_values_flat(amatrix_max_width * amatrix_height, 0);
    std::vector<uint64_t> amatrix_indices_flat(amatrix_width * amatrix_height, 0);

    // Go to start of file (after height and width declarations)
    amatrix_file.clear();
    amatrix_file.seekg(0, std::ios::beg);
    int skr = skip_lines(amatrix_file, 2);

    while (std::getline(amatrix_file, amatrix_line)) {
        std::istringstream token_stream(amatrix_line);
        std::string token;
        std::getline(token_stream, token, ' ');
        uint64_t amatrix_row = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));

        std::getline(token_stream, token, ' ');
        uint64_t amatrix_column = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));

        std::getline(token_stream, token, ' ');
        double amatrix_value = std::strtof(token.c_str(), const_cast<char**>(&end_ptr));

        uint64_t new_col = 0;
        bool col_found = false;

        for (uint64_t r_col = 0; r_col < amatrix_width; ++r_col) {
            if (amatrix_values_flat[amatrix_row * amatrix_width + r_col] == 0) {
                new_col = r_col;
                col_found = true;
                break;
            }
        }

        amatrix_values_flat[amatrix_row * amatrix_width + new_col] = amatrix_value;
        amatrix_indices_flat[amatrix_row * amatrix_width + new_col] = amatrix_column;
    }

    std::vector<double> bvector = {4.3, 5.0, 3.0};

    std::vector<std::vector<double>> r_values(amatrix_height, std::vector<double>());
    std::vector<std::vector<uint64_t>> r_indices(amatrix_height, std::vector<uint64_t>());
    std::vector<uint64_t> r_row_lengths(amatrix_height, 0);
    std::vector<double> r_row_values(amatrix_height, 0);
    std::vector<uint64_t> r_row_indices(amatrix_height, 0);
    uint64_t max_width = 0;

    if (r_values.empty() || r_indices.empty() || r_row_lengths.empty()) {
        amatrix_height = 0;
    }

#pragma omp parallel for default(none) shared(amatrix_width, amatrix_height, amatrix_values_flat, amatrix_indices_flat, bvector, r_values, r_indices, r_row_lengths, r_row_values, r_row_indices, max_width) schedule(static)
    for (uint64_t r_row_i = 0; r_row_i < amatrix_height; r_row_i++) {
        uint64_t r_column_counter = 0;
        uint64_t a_column_i = 0;
        uint64_t b_column_i = 0;
        double res_sum = 0.0F;

        while (a_column_i < amatrix_width && b_column_i < bvector.size()) {
            if (amatrix_indices_flat[r_row_i * amatrix_width + a_column_i] == b_column_i) {
                res_sum += amatrix_values_flat[r_row_i * amatrix_width + a_column_i] * bvector[b_column_i];
                a_column_i++;
                b_column_i++;
            } else if (amatrix_indices_flat[r_row_i * amatrix_width + a_column_i] > b_column_i) {
                b_column_i++;
            } else {
                a_column_i++;
            }
        }

        if (res_sum != 0.0) {
            #pragma omp critical
            {
                r_row_values[r_column_counter] = res_sum;
                r_column_counter++;
            }
        }

        #pragma omp critical
        {
            if (r_column_counter > max_width) {
                max_width = r_column_counter;
            }
        }

        #pragma omp critical
        {
            r_values[r_row_i].assign(r_row_values.begin(), r_row_values.begin() + r_column_counter);
            r_indices[r_row_i].assign(r_row_indices.begin(), r_row_indices.begin() + r_column_counter);
            r_row_lengths[r_row_i] = r_column_counter;
        }
    }

    uint64_t result_height = amatrix_height;

    std::vector<std::vector<double>> result_values(result_height, std::vector<double>(max_width, 0));
    std::vector<std::vector<uint64_t>> result_indices(result_height, std::vector<uint64_t>(max_width, 0));

    if (!result_values.empty() && !result_indices.empty()) {
        for (uint64_t x_row_i = 0; x_row_i < result_height; x_row_i++) {
            auto values_dest = result_values[x_row_i].begin();
            auto indices_dest = result_indices[x_row_i].begin();

            std::copy(r_values[x_row_i].begin(), r_values[x_row_i].end(), values_dest);
            std::copy(r_indices[x_row_i].begin(), r_indices[x_row_i].end(), indices_dest);
        }
    }

    printf("[FREE] Freeing used memory ...\n");
    for (int i = 0; i < result_height; ++i) {
        for (int j = 0; j < max_width; ++j) {
            std::cout << result_values[i][j] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
