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
    long input_width = 0;
    long input_height = 0;

    while (std::getline(matrix_file, line)) {
        if (line_count > 1) {
            break;
        } else if (line_count == 0) {
            errno = 0;
            input_height = std::strtol(line.c_str(), const_cast<char**>(&end_ptr), 10);
        } else if (line_count == 1) {
            errno = 0;
            input_width = std::strtol(line.c_str(), const_cast<char**>(&end_ptr), 10);
        }
        ++line_count;
    }

    printf("[SCAN] Scanning matrix %s ...\n", matrix_path);

    std::string matrix_line;
    line_count = 4;

    uint64_t current_max_width = 0;
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
            if (current_max_width < count_used) {
                current_max_width = count_used;
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

    if (current_max_width <= 0) {
        current_max_width = static_cast<uint64_t>(input_width);
    }

    printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", input_width, current_max_width);
    printf("[INIT] Allocating %lu bytes of memory for matrix %s\n", (4 * current_max_width * input_height) + (8 * current_max_width * input_height), matrix_path);

    uint64_t real_width = input_width;
    uint64_t width = current_max_width;
    uint64_t height = input_height;
    float* values = new float[current_max_width * input_height];
    uint64_t* indices = new uint64_t[current_max_width * input_height];

    for (uint64_t run_row = 0; run_row < height; ++run_row) {
        for (uint64_t run_col = 0; run_col < width; ++run_col) {
            indices[run_row * width + run_col] = 0;
            values[run_row * width + run_col] = 0;
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

        for (uint64_t r_col = 0; r_col < width; ++r_col) {
            if (values[row * width + r_col] == 0) {
                new_col = r_col;
                col_found = true;
                break;
            }
        }

        values[row * width + new_col] = value;
        indices[row * width + new_col] = column;

        ++line_count;
    }

    std::cout << "Ellpack format: " << std::endl;
    for (uint64_t i = 0; i < height; ++i) {
        for (uint64_t j = 0; j < real_width; ++j) {
            std::cout << values[i * real_width + j] << " ";
        }
        std::cout << std::endl;
    }

    matrix_file.close();


    std::vector<double> b = {4.3, 5.0, 3.0};
    EllpackMatrix* result = new EllpackMatrix();
    std::vector<double> resultvec (0, height);

    result->height = height;
    float** r_values = new float*[height]();
    uint64_t** r_indices = new uint64_t*[height]();
    uint64_t* r_row_lengths = new uint64_t[height]();
    float* r_row_values = new float[height]();
    uint64_t* r_row_indices = new uint64_t[height]();
    uint64_t max_width = 0;

    if (!r_values || !r_indices || !r_row_lengths) {
        result->height = 0;
        resultvec.clear();
    }
//vector
for (uint64_t r_row_i = 0; r_row_i < resultvec.size(); r_row_i++) {
        uint64_t r_column_counter = 0;
        uint64_t a_column_i = 0;
        uint64_t b_column_i = 0;
        float res_sum = 0.0F;

        while (a_column_i < width && b_column_i < b.size()) {
            if (indices[r_row_i * width + a_column_i] == b_column_i) {
                res_sum += values[r_row_i * width + a_column_i] * b[b_column_i];
                a_column_i++;
                b_column_i++;
            } else if (indices[r_row_i * width + a_column_i] > b_column_i) {
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
            resultvec.resize(r_row_i + 1);
            break;
        }

        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(uint64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }



//ellpack
    for (uint64_t r_row_i = 0; r_row_i < result->height; r_row_i++) {
        uint64_t r_column_counter = 0;
        uint64_t a_column_i = 0;
        uint64_t b_column_i = 0;
        float res_sum = 0.0F;

        while (a_column_i < width && b_column_i < b.size()) {
            if (indices[r_row_i * width + a_column_i] == b_column_i) {
                res_sum += values[r_row_i * width + a_column_i] * b[b_column_i];
                a_column_i++;
                b_column_i++;
            } else if (indices[r_row_i * width + a_column_i] > b_column_i) {
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
            result->height = r_row_i + 1;
            break;
        }

        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(uint64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }

    //vector
    /*
        uint64_t real_width;
    uint64_t height;
    uint64_t width;
    float* values;
     indices;*/
    
    uint64_t resultvecwidth = max_width;

    float* resultvecvalues = new float[resultvec.size() * resultvecwidth]();
    uint64_t* resultvecindices = new uint64_t[resultvec.size() * resultvecwidth]();
    if (resultvecvalues && resultvecindices) {
        for (uint64_t x_row_i = 0; x_row_i < resultvec.size(); x_row_i++) {
            memcpy(resultvecvalues + x_row_i * resultvecwidth, r_values[x_row_i], r_row_lengths[x_row_i] * sizeof(float));
            memcpy(resultvecindices + x_row_i * resultvecwidth, r_indices[x_row_i], r_row_lengths[x_row_i] * sizeof(uint64_t));
        }
    }
    //ellpack

    result->width = max_width;

    result->values = new float[result->height * result->width]();
    result->indices = new uint64_t[result->height * result->width]();
    if (result->values && result->indices) {
        for (uint64_t x_row_i = 0; x_row_i < result->height; x_row_i++) {
            memcpy(result->values + x_row_i * result->width, r_values[x_row_i], r_row_lengths[x_row_i] * sizeof(float));
            memcpy(result->indices + x_row_i * result->width, r_indices[x_row_i], r_row_lengths[x_row_i] * sizeof(uint64_t));
        }
    }

    for (uint64_t x_xow_i = 0; x_xow_i < result->height; x_xow_i++) {
        delete[] r_values[x_xow_i];
        delete[] r_indices[x_xow_i];
    }
    delete[] r_values;
    delete[] r_indices;

    delete[] r_row_values;
    delete[] r_row_indices;
 
//vector
 std::ofstream out_file2("out2.mat");
    // Print the result in reduced coordinate schema
    for (uint64_t run_row = 0; run_row < resultvec.size(); ++run_row) {
        for (uint64_t run_col = 0; run_col < resultvecwidth; ++run_col) {
            uint64_t index_entry = resultvecindices[run_row * resultvecwidth + run_col];
            float value_entry = resultvecvalues[run_row * resultvecwidth + run_col];
            if (value_entry != 0) {
                uint64_t real_column = index_entry;
                out_file2 << run_row << ' ' << real_column << ' ' << std::scientific << value_entry;
                if (run_row < (resultvec.size() - 1) || run_col < (resultvecwidth - 1)) {
                    out_file2 << '\n';
                }
            }
        }
    }

    out_file2.close();

//ellpack
        std::ofstream out_file("out1.mat");
    // Print the result in reduced coordinate schema
    for (uint64_t run_row = 0; run_row < result->height; ++run_row) {
        for (uint64_t run_col = 0; run_col < result->width; ++run_col) {
            uint64_t index_entry = result->indices[run_row * result->width + run_col];
            float value_entry = result->values[run_row * result->width + run_col];
            if (value_entry != 0) {
                uint64_t real_column = index_entry;
                out_file << run_row << ' ' << real_column << ' ' << std::scientific << value_entry;
                if (run_row < (result->height - 1) || run_col < (result->width - 1)) {
                    out_file << '\n';
                }
            }
        }
    }

    out_file.close();


    std::cout << "[FREE] Freeing used memory ...\n";


    return result;
}

int main(int argc, char** argv) {
    EllpackMatrix* a = parse_matrix("a.mat");

    return 0;
}
