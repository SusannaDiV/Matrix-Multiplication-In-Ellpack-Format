#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

int skip_lines(std::ifstream& file, unsigned long long num) {
    std::string line;
    for (unsigned long long run = 0; run < num; ++run) {
        if (!std::getline(file, line)) {
            return -1;
        }
    }
    return 0;
}

int main() {
    std::ifstream amatrix_file("a.mat");
    std::string amatrix_line;
    unsigned long long line_count = 0;
    const char* end_ptr;
    long amatrix_width = 0;
    long amatrix_height = 0;

    while (std::getline(amatrix_file, amatrix_line)) {
        if (line_count > 1) {
            break;
        } else if (line_count == 0) {
            errno = 0;
            amatrix_height = std::strtol(amatrix_line.c_str(), const_cast<char**>(&end_ptr), 10);
        } else if (line_count == 1) {
            errno = 0;
            amatrix_width = std::strtol(amatrix_line.c_str(), const_cast<char**>(&end_ptr), 10);
        }
        ++line_count;
    }

    printf("[SCAN] Scanning matrix a.mat ...\n");

    line_count = 4;

    uint64_t amatrix_max_width = 0;
    uint64_t amatrix_current_row = 0;
    uint64_t amatrix_count_used = 0;

    // Creating vectors to store values and indices
    std::vector<std::vector<float>> amatrix_values(amatrix_height);
    std::vector<std::vector<uint64_t>> amatrix_indices(amatrix_height);

    while (std::getline(amatrix_file, amatrix_line)) {
        std::istringstream token_stream(amatrix_line);
        std::string token;

        std::getline(token_stream, token, ';');
        errno = 0;
        long amatrix_row_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t amatrix_row = static_cast<uint64_t>(amatrix_row_raw);

        if (amatrix_row > amatrix_current_row) {
            amatrix_current_row = amatrix_row;
            if (amatrix_max_width < amatrix_count_used) {
                amatrix_max_width = amatrix_count_used;
            }
            amatrix_count_used = 0;
        }

        std::getline(token_stream, token, ';');
        errno = 0;
        long amatrix_column_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t amatrix_column = static_cast<uint64_t>(amatrix_column_raw);
        ++amatrix_count_used;
        ++line_count;

        // Add value and index to corresponding row vectors
        amatrix_values[amatrix_row].push_back(0);  // Initializing with 0
        amatrix_indices[amatrix_row].push_back(amatrix_column);
    }

    if (amatrix_max_width <= 0) {
        amatrix_max_width = static_cast<uint64_t>(amatrix_width);
    }

    printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", amatrix_width, amatrix_max_width);
    printf("[INIT] Allocating %lu bytes of memory for matrix a.mat\n", (4 * amatrix_max_width * amatrix_height) + (8 * amatrix_max_width * amatrix_height));

    // Displaying the values and indices
    for (uint64_t i = 0; i < amatrix_height; ++i) {
        printf("Row %lu: ", i);
        for (size_t j = 0; j < amatrix_values[i].size(); ++j) {
            printf("(%lu, %.2f) ", amatrix_indices[i][j], amatrix_values[i][j]);
        }
        printf("\n");
    }

    amatrix_file.close();
    return 0;
}
