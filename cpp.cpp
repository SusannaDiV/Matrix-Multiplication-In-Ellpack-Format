#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring> 
struct EllpackMatrix {
    uint64_t real_width;
    uint64_t height;
    uint64_t width;
    float* values;
    uint64_t* indices;
};

EllpackMatrix* make_ellpack(uint64_t real_width, uint64_t height, uint64_t width) {
    EllpackMatrix* ellpack = new EllpackMatrix();
    ellpack->real_width = real_width;
    ellpack->width = width;
    ellpack->height = height;
    ellpack->values = new float[width * height];
    ellpack->indices = new uint64_t[width * height];
    return ellpack;
}

void matr_mult_ellpack(const void* a, std::vector<double> &b, void* result) {
    EllpackMatrix* r = static_cast<EllpackMatrix*>(result);
    const EllpackMatrix* ax = static_cast<const EllpackMatrix*>(a);
    r->height = ax->height;
    float** r_values = new float*[ax->height]();
    uint64_t** r_indices = new uint64_t*[ax->height]();
    uint64_t* r_row_lengths = new uint64_t[ax->height]();
    float* r_row_values = new float[ax->height]();
    uint64_t* r_row_indices = new uint64_t[ax->height]();
    uint64_t max_width = 0;

    if (!r_values || !r_indices || !r_row_lengths) {
        r->height = 0;
    }
    std::vector<double> bx(b.begin(), b.end());

    for (uint64_t r_row_i = 0; r_row_i < r->height; r_row_i++) {
        uint64_t r_column_counter = 0;
        uint64_t a_column_i = 0;
        uint64_t b_column_i = 0;
        float res_sum = 0.0F;

        while (a_column_i < ax->width && b_column_i < bx.size()) {
            if (ax->indices[r_row_i * ax->width + a_column_i] == b_column_i) {
                res_sum += ax->values[r_row_i * ax->width + a_column_i] * bx[b_column_i];
                a_column_i++;
                b_column_i++;
            } else if (ax->indices[r_row_i * ax->width + a_column_i] > b_column_i) {
                b_column_i++;
            } else {
                a_column_i++;
            }
        }

        if (res_sum != 0.0) {
            r_row_values[r_column_counter] = res_sum;
            r_column_counter++;
        }

        if (r_column_counter > max_width) {
            max_width = r_column_counter;
        }

        r_values[r_row_i] = new float[r_column_counter]();
        r_indices[r_row_i] = new uint64_t[r_column_counter]();

        if (!r_values[r_row_i] || !r_indices[r_row_i]) {
            r->height = r_row_i + 1;
            break;
        }

        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(uint64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }

    r->width = max_width;

    r->values = new float[r->height * r->width]();
    r->indices = new uint64_t[r->height * r->width]();
    if (r->values && r->indices) {
        for (uint64_t x_row_i = 0; x_row_i < r->height; x_row_i++) {
            memcpy(r->values + x_row_i * r->width, r_values[x_row_i], r_row_lengths[x_row_i] * sizeof(float));
            memcpy(r->indices + x_row_i * r->width, r_indices[x_row_i], r_row_lengths[x_row_i] * sizeof(uint64_t));
        }
    }

    for (uint64_t x_xow_i = 0; x_xow_i < r->height; x_xow_i++) {
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

EllpackMatrix* parse_matrix(const char* matrix_path) {
    std::ifstream matrix_file(matrix_path);
    std::string line;
    unsigned long long line_count = 0;
    const char* end_ptr;
    long width = 0;
    long height = 0;

    while (std::getline(matrix_file, line)) {
        if (line_count > 1) {
            break;
        } else if (line_count == 0) {
            errno = 0;
            height = std::strtol(line.c_str(), const_cast<char**>(&end_ptr), 10);
        } else if (line_count == 1) {
            errno = 0;
            width = std::strtol(line.c_str(), const_cast<char**>(&end_ptr), 10);
        }
        ++line_count;
    }

    printf("[SCAN] Scanning matrix %s ...\n", matrix_path);

    std::string matrix_line;
    line_count = 4;

    uint64_t max_width = 0;
    uint64_t current_row = 0;
    uint64_t count_used = 0;

    while (std::getline(matrix_file, matrix_line)) {
        std::istringstream token_stream(matrix_line);
        std::string token;

        std::getline(token_stream, token, ';');
        errno = 0;
        long row_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t row = static_cast<uint64_t>(row_raw);

        if (row > current_row) {
            current_row = row;
            if (max_width < count_used) {
                max_width = count_used;
            }
            count_used = 0;
        }

        std::getline(token_stream, token, ';');
        errno = 0;
        long column_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t column = static_cast<uint64_t>(column_raw);
        ++count_used;
        ++line_count;
    }

    if (max_width <= 0) {
        max_width = static_cast<uint64_t>(width);
    }

    printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", width, max_width);
    printf("[INIT] Allocating %lu bytes of memory for matrix %s\n", (4 * max_width * height) + (8 * max_width * height), matrix_path);

    EllpackMatrix* matrix = make_ellpack(static_cast<uint64_t>(width), static_cast<uint64_t>(height), max_width);


    for (uint64_t run_row = 0; run_row < matrix->height; ++run_row) {
        for (uint64_t run_col = 0; run_col < matrix->width; ++run_col) {
            matrix->indices[run_row * matrix->width + run_col] = 0;
            matrix->values[run_row * matrix->width + run_col] = 0;
        }
    }

    printf("[INIT] Reading data of matrix %s into memory\n", matrix_path);

    // Go to start of file (after height and width declarations)
    matrix_file.clear();
    matrix_file.seekg(0, std::ios::beg);
    int skr = skip_lines(matrix_file, 3);
    line_count = 3;

    while (std::getline(matrix_file, matrix_line)) {
        std::istringstream token_stream(matrix_line);
        std::string token;
        std::getline(token_stream, token, ' ');
        errno = 0;
        long row_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t row = static_cast<uint64_t>(row_raw);

        std::getline(token_stream, token, ' ');
        errno = 0;
        long column_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t column = static_cast<uint64_t>(column_raw);

        std::getline(token_stream, token, ' ');
        errno = 0;
        float value = std::strtof(token.c_str(), const_cast<char**>(&end_ptr));

        uint64_t new_col = 0;
        bool col_found = false;

        for (uint64_t r_col = 0; r_col < matrix->width; ++r_col) {
            if (matrix->values[row * matrix->width + r_col] == 0) {
                new_col = r_col;
                col_found = true;
                break;
            }
        }

        matrix->values[row * matrix->width + new_col] = value;
        matrix->indices[row * matrix->width + new_col] = column;

        ++line_count;
    }

    for (uint64_t i = 0; i < matrix->height; ++i) {
        for (uint64_t j = 0; j < matrix->real_width; ++j) {
            std::cout << matrix->values[i * matrix->real_width + j] << " ";
        }
        std::cout << std::endl;
    }

    matrix_file.close();
    return matrix;
}

























void write_matrix(EllpackMatrix* matrix, const char* out_path) {
    std::ofstream out_file(out_path);
    // Print the matrix in reduced coordinate schema
    for (uint64_t run_row = 0; run_row < matrix->height; ++run_row) {
        for (uint64_t run_col = 0; run_col < matrix->width; ++run_col) {
            uint64_t index_entry = matrix->indices[run_row * matrix->width + run_col];
            float value_entry = matrix->values[run_row * matrix->width + run_col];
            if (value_entry != 0) {
                uint64_t real_column = index_entry;
                out_file << run_row << ' ' << real_column << ' ' << std::scientific << value_entry;
                if (run_row < (matrix->height - 1) || run_col < (matrix->width - 1)) {
                    out_file << '\n';
                }
            }
        }
    }

    out_file.close();
}

int main(int argc, char** argv) {
    EllpackMatrix* amatrix = parse_matrix("a.mat");

    std::vector<double> bvector = {4.3, 5.0, 3.0};

    EllpackMatrix* result = new EllpackMatrix();
    matr_mult_ellpack(amatrix, bvector, result);
    for (int i = 0; i < result->height; ++i) {
        std::cout << result->values[i] << std::endl;
    }
    write_matrix(result, "out1.mat");

    std::cout << "[FREE] Freeing used memory ...\n";

    return 0;
}
