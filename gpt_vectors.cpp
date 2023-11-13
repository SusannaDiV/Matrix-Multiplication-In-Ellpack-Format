#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

void matr_mult_ellpack(uint64_t ax_width, uint64_t ax_height, float* ax_values, uint64_t* ax_indices,
                       std::vector<double>& b, uint64_t& result_height, uint64_t& result_width,
                       float*& result_values, uint64_t*& result_indices) {
    result_height = ax_height;
    float** r_values = new float*[ax_height]();
    uint64_t** r_indices = new uint64_t*[ax_height]();
    uint64_t* r_row_lengths = new uint64_t[ax_height]();
    float* r_row_values = new float[ax_height]();
    uint64_t* r_row_indices = new uint64_t[ax_height]();
    uint64_t max_width = 0;

    if (!r_values || !r_indices || !r_row_lengths) {
        result_height = 0;
    }
    std::vector<double> bx(b.begin(), b.end());

    #pragma omp parallel for default(none) shared(ax_width, ax_height, ax_values, ax_indices, bx, r_values, r_indices, r_row_lengths, r_row_values, r_row_indices, result_height, max_width) schedule(static)
    for (uint64_t r_row_i = 0; r_row_i < result_height; r_row_i++) {
        uint64_t r_column_counter = 0;
        uint64_t a_column_i = 0;
        uint64_t b_column_i = 0;
        float res_sum = 0.0F;

        while (a_column_i < ax_width && b_column_i < bx.size()) {
            if (ax_indices[r_row_i * ax_width + a_column_i] == b_column_i) {
                res_sum += ax_values[r_row_i * ax_width + a_column_i] * bx[b_column_i];
                a_column_i++;
                b_column_i++;
            } else if (ax_indices[r_row_i * ax_width + a_column_i] > b_column_i) {
                b_column_i++;
            } else {
                a_column_i++;
            }
        }

        if (res_sum != 0.0) {
            r_row_values[r_column_counter] = res_sum;
            r_column_counter++;
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
            if (!r_values[r_row_i] || !r_indices[r_row_i]) {
                result_height = r_row_i + 1;
            }
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
    #pragma omp parallel for default(none) shared(result_values, result_indices, r_values, r_indices, r_row_lengths, result_height, result_width) schedule(static)
    for (uint64_t x_row_i = 0; x_row_i < result_height; x_row_i++) {
        memcpy(result_values + x_row_i * result_width, r_values[x_row_i], r_row_lengths[x_row_i] * sizeof(float));
        memcpy(result_indices + x_row_i * result_width, r_indices[x_row_i], r_row_lengths[x_row_i] * sizeof(uint64_t));
    }

    #pragma omp parallel for default(none) shared(r_values, r_indices, result_height) schedule(static)
    for (uint64_t x_xow_i = 0; x_xow_i < result_height; x_xow_i++) {
        delete[] r_values[x_xow_i];
        delete[] r_indices[x_xow_i];
    }
    delete[] r_values;
    delete[] r_indices;

    delete[] r_row_values;
    delete[] r_row_indices;
}
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

void write_matrix(uint64_t result_height, uint64_t result_width,
                  float* result_values, uint64_t* result_indices,
                  const char* out_path) {
    std::ofstream out_file(out_path);
    // Print the matrix in reduced coordinate schema
    for (uint64_t run_row = 0; run_row < result_height; ++run_row) {
        for (uint64_t run_col = 0; run_col < result_width; ++run_col) {
            uint64_t index_entry = result_indices[run_row * result_width + run_col];
            float value_entry = result_values[run_row * result_width + run_col];
            if (value_entry != 0) {
                uint64_t real_column = index_entry;
                out_file << run_row << ' ' << real_column << ' ' << std::scientific << value_entry;
                if (run_row < (result_height - 1) || run_col < (result_width - 1)) {
                    out_file << '\n';
                }
            }
        }
    }

    out_file.close();
}





int main(int argc, char** argv) {
    std::ifstream amatrix_file("a.mat");
    std::string amatrix_line;
    unsigned long long line_count = 0;
    const char* end_ptr;
    long amatrix_width = 0;
    long amatrix_height = 0;
    long tot=0;
/*
    std::getline(amatrix_file, amatrix_line);
    std::istringstream dim_stream(amatrix_line);
    dim_stream >> amatrix_height >> amatrix_width >> tot;
    std::cout << "hieght: " << amatrix_height << std::endl;
    std::cout << "width: " << amatrix_width << std::endl;
*/
    while (std::getline(amatrix_file, amatrix_line)) {
        if (line_count > 1) {
            break;
        } else if (line_count == 0) {
            errno = 0;
            amatrix_height = std::strtol(amatrix_line.c_str(), const_cast<char**>(&end_ptr), 10);
            std::cout << "hieght: " << amatrix_height << std::endl;
        } else if (line_count == 1) {
            errno = 0;
            amatrix_width = std::strtol(amatrix_line.c_str(), const_cast<char**>(&end_ptr), 10);
            std::cout << "width: " << amatrix_width << std::endl;
        }
        ++line_count;
    }

    printf("[SCAN] Scanning matrix a.mat ...\n");

    line_count = 4;

    uint64_t amatrix_max_width = 0;
    uint64_t amatrix_current_row = 0;
    uint64_t amatrix_count_used = 0;

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
        ++line_count;
    }

    if (amatrix_max_width <= 0) {
        amatrix_max_width = static_cast<uint64_t>(amatrix_width);
    }

    printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", amatrix_width, amatrix_max_width);

    float* amatrix_values = new float[amatrix_width * amatrix_height];
    uint64_t* amatrix_indices = new uint64_t[amatrix_width * amatrix_height];

    for (uint64_t run_row = 0; run_row < amatrix_height; ++run_row) {
        for (uint64_t run_col = 0; run_col < amatrix_width; ++run_col) {
            amatrix_indices[run_row * amatrix_width + run_col] = 0;
            amatrix_values[run_row * amatrix_width + run_col] = 0;
        }
    }

    printf("[INIT] Reading data of matrix a.mat into memory\n");

    // Go to start of file (after height and width declarations)
    amatrix_file.clear();
    amatrix_file.seekg(0, std::ios::beg);
    int skr = skip_lines(amatrix_file, 3);
    line_count = 3;

    while (std::getline(amatrix_file, amatrix_line)) {
        std::istringstream token_stream(amatrix_line);
        std::string token;
        std::getline(token_stream, token, ' ');
        uint64_t amatrix_row = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));

        std::getline(token_stream, token, ' ');
        uint64_t amatrix_column = static_cast<uint64_t>(std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10));

        std::getline(token_stream, token, ' ');
        float amatrix_value = std::strtof(token.c_str(), const_cast<char**>(&end_ptr));

        uint64_t new_col = 0;
        bool col_found = false;

        for (uint64_t r_col = 0; r_col < amatrix_width; ++r_col) {
            if (amatrix_values[amatrix_row * amatrix_width + r_col] == 0) {
                new_col = r_col;
                col_found = true;
                break;
            }
        }

        amatrix_values[amatrix_row * amatrix_width + new_col] = amatrix_value;
        amatrix_indices[amatrix_row * amatrix_width + new_col] = amatrix_column;

        ++line_count;
    }
/*
    for (uint64_t i = 0; i < amatrix_height; ++i) {
        for (uint64_t j = 0; j < amatrix_max_width; ++j) {
            std::cout << amatrix_values[i * amatrix_max_width + j] << " ";
        }
        std::cout << std::endl;
    }

    amatrix_file.close();
*/
    std::vector<double> bvector = {4.3, 5.0, 3.0};

    uint64_t result_height, result_width;
    float* result_values;
    uint64_t* result_indices;

    matr_mult_ellpack(amatrix_width, amatrix_height, amatrix_values, amatrix_indices,
                       bvector, result_height, result_width, result_values, result_indices);

    write_matrix(result_height, result_width, result_values, result_indices, "out1.mat");
   std::cout << "[FREE] Freeing used memory ...\n";
    for (int i=0; i<result_height; ++i) {
        std::cout << result_values[i] << std::endl;
    }
    // Free memory
        delete[] amatrix_values;
    delete[] amatrix_indices;
        delete[] result_values;
    delete[] result_indices;

 
    return 0;
}
