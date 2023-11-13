#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
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
    long numNonZero = 0;

    amatrix_file >> amatrix_width >> amatrix_height >> numNonZero;

    printf("[INFO] Matrix dimensions: %lu x %lu, Non-zero elements: %lu\n", amatrix_height, amatrix_width, numNonZero);

    std::vector<std::vector<float>> amatrix_values(amatrix_height);
    std::vector<std::vector<uint64_t>> amatrix_indices(amatrix_height);

    for (uint64_t i = 0; i < amatrix_height; ++i) {
        amatrix_values[i].resize(numNonZero, 0.0);
        amatrix_indices[i].resize(numNonZero, 0);
    }

    printf("[SCAN] Completed, Initializing matrix...\n");

    while (std::getline(amatrix_file, amatrix_line)) {
        std::istringstream token_stream(amatrix_line);
        std::string token;
        
        std::getline(token_stream, token, ' ');
        uint64_t amatrix_column = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));

        std::getline(token_stream, token, ' ');
        uint64_t amatrix_row = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));

        std::getline(token_stream, token, ' ');
        float amatrix_value = std::strtof(token.c_str(), const_cast<char**>(&end_ptr));

        // Adjust indices to 0-based
        --amatrix_column;
        --amatrix_row;

        // Find the first available position
        for (uint64_t i = 0; i < numNonZero; ++i) {
            if (amatrix_values[amatrix_row][i] == 0) {
                amatrix_values[amatrix_row][i] = amatrix_value;
                amatrix_indices[amatrix_row][i] = amatrix_column;
                break;
            }
        }
    }

    // Print the compressed matrix
    printf("[PRINT] Content of compressed matrix:\n");
    for (uint64_t i = 0; i < amatrix_height; ++i) {
        printf("Row %lu: ", i);
        for (uint64_t j = 0; j < numNonZero; ++j) {
            printf("(%lu, %.2f) ", amatrix_indices[i][j], amatrix_values[i][j]);
        }
        printf("\n");
    }


    std::vector<double> bvector = {4.3, 5.0, 3.0};

    uint64_t result_height, result_width;
    float* result_values;
    uint64_t* result_indices;

    result_height = amatrix_height;
    float** r_values = new float*[amatrix_height]();
    uint64_t** r_indices = new uint64_t*[amatrix_height]();
    uint64_t* r_row_lengths = new uint64_t[amatrix_height]();
    float* r_row_values = new float[amatrix_height]();
    uint64_t* r_row_indices = new uint64_t[amatrix_height]();
    uint64_t max_width = 0;

    if (!r_values || !r_indices || !r_row_lengths) {
        result_height = 0;
    }

#pragma omp parallel for default(none) shared(amatrix_width, amatrix_height, amatrix_values, amatrix_indices, bvector, r_values, r_indices, r_row_lengths, r_row_values, r_row_indices, result_height, max_width) schedule(static)
    for (uint64_t r_row_i = 0; r_row_i < result_height; r_row_i++) {
        uint64_t r_column_counter = 0;
        uint64_t a_column_i = 0;
        uint64_t b_column_i = 0;
        float res_sum = 0.0F;

        while (a_column_i < amatrix_width && b_column_i < bvector.size()) {
            if (amatrix_indices[r_row_i * amatrix_width + a_column_i] == b_column_i) {
                res_sum += amatrix_values[r_row_i * amatrix_width + a_column_i] * bvector[b_column_i];
                a_column_i++;
                b_column_i++;
            } else if (amatrix_indices[r_row_i * amatrix_width + a_column_i] > b_column_i) {
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
            r_values[r_row_i] = new float[r_column_counter]();
            r_indices[r_row_i] = new uint64_t[r_column_counter]();
        }

        #pragma omp critical
        {
            memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
            memcpy(r_indices[r_row_i], r_row_indices, sizeof(uint64_t) * r_column_counter);
            r_row_lengths[r_row_i] = r_column_counter;
        }
    }

    result_width = max_width;

    result_values = new float[result_height * result_width]();
    result_indices = new uint64_t[result_height * result_width]();
    if (result_values && result_indices) {
        for (uint64_t x_row_i = 0; x_row_i < result_height; x_row_i++) {
            memcpy(result_values + x_row_i * result_width, r_values[x_row_i], r_row_lengths[x_row_i] * sizeof(float));
            memcpy(result_indices + x_row_i * result_width, r_indices[x_row_i], r_row_lengths[x_row_i] * sizeof(uint64_t));
        }
    }
    printf("[FREE] Freeing used memory ...\n");
    for (int i = 0; i < result_height; ++i) {
        std::cout << result_values[i] << std::endl;
    }
    return 0;
}
