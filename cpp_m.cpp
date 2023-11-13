#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring> 

// Function to read vector from file
std::vector<double> readVectorFromFile(const char* filename) {
    FILE* fp_vec = fopen(filename, "rb+");  // Read vector file
    int vectorSize;
    if (fscanf(fp_vec, "%d", &vectorSize) != 1) {
        fclose(fp_vec);
        return {};
    }
    std::vector<double> vectorValues;
    double value;
    for (int i = 0; i < vectorSize; ++i) {
        if (fscanf(fp_vec, "%lf", &value) != 1) {
            fclose(fp_vec);
            return {};
        }
        vectorValues.push_back(value);
    }
    fclose(fp_vec);
    return vectorValues;
}





int main(int argc, char** argv) {
//    FILE *fp_mtx;
//    fp_mtx = fopen("a", "rb+");  // Read matrix file
    std::vector<double> vector = readVectorFromFile("b");
    FILE* fp_mtx = fopen("a", "r"); 
    long total_rows, total_cols, total_non_null;
    // Read the first line of the matrix file
    if (fscanf(fp_mtx, "%ld %ld %ld", &total_rows, &total_cols, &total_non_null) != 3) {
        fclose(fp_mtx);
        std::cout<<"a";
    }
    unsigned long long line_count = 0;
    const char* end_ptr;
    uint64_t max_width = 0;
    uint64_t current_row = 0;
    uint64_t count_used = 0;
       char line[256];  // Adjust the buffer size accordingly
    while (fgets(line, sizeof(line), fp_mtx)) {
        char* token = strtok(line, " ");
        if (token != nullptr) {
            errno = 0;
            long row_raw = std::strtol(token, const_cast<char**>(&end_ptr), 10);
            uint64_t row = static_cast<uint64_t>(row_raw);

            // Update current row and count of used elements
            if (row > current_row) {
                current_row = row;
                if (max_width < count_used) {
                    max_width = count_used;
                }
                count_used = 0;
            }

            // Increment count of used elements and line count
            ++count_used;
            ++line_count;
        }
    }

    // Ensure max_width is not less than 0
    if (max_width <= 0) {
        max_width = static_cast<uint64_t>(total_cols);
    }
              printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", total_cols, max_width);

//debug
    uint64_t real_width = total_cols;
    uint64_t width = max_width;
    uint64_t height = total_rows;
    float* values = new float[max_width * total_rows];
    uint64_t* indeces = new uint64_t[max_width * total_rows];



    // Initialize the values and indices arrays
    for (uint64_t i = 0; i < max_width * total_rows; ++i) {
        values[i] = 0.0f;
        indeces[i] = 0;
    }

    // Go back to the start of the file
    fseek(fp_mtx, 0, SEEK_SET);

    // Skip the first line
    fgets(line, sizeof(line), fp_mtx);

    // Read the matrix data and populate values and indices
    while (fgets(line, sizeof(line), fp_mtx)) {
        char* token = strtok(line, " ");
        if (token != nullptr) {
            errno = 0;
            long row_raw = std::strtol(token, const_cast<char**>(&end_ptr), 10);
            uint64_t row = static_cast<uint64_t>(row_raw);

            // Read column information
            token = strtok(nullptr, " ");
            if (token != nullptr) {
                errno = 0;
                long column_raw = std::strtol(token, const_cast<char**>(&end_ptr), 10);
                uint64_t column = static_cast<uint64_t>(column_raw);

                // Read value information
                token = strtok(nullptr, " ");
                if (token != nullptr) {
                    errno = 0;
                    float value = std::strtof(token, const_cast<char**>(&end_ptr));

                    // Find an empty column for the element
                    uint64_t new_col = 0;
                    for (uint64_t r_col = 0; r_col < width; ++r_col) {
                        if (values[row * width + r_col] == 0) {
                            new_col = r_col;
                            break;
                        }
                    }

                    // Update values and indices arrays
                    values[row * width + new_col] = value;
                    indeces[row * width + new_col] = column;
                }
            }
        }
    }

    // Close the matrix file
    fclose(fp_mtx);

    // Print the matrix values for verification
    for (uint64_t i = 0; i < height; ++i) {
        for (uint64_t j = 0; j < real_width; ++j) {
            std::cout << values[i * real_width + j] << " ";
        }
        std::cout << std::endl;
    }

    //PERFECT! :D

    //Moltoplicazione matrice vettore
    std::vector<double> result(0, height);

     float** r_values = new float*[height]();
    uint64_t** r_indices = new uint64_t*[height]();
    uint64_t* r_row_lengths = new uint64_t[height]();
    float* r_row_values = new float[height]();
    uint64_t* r_row_indices = new uint64_t[height]();

    if (!r_values || !r_indices || !r_row_lengths) {
        result.clear();
    }

    std::vector<double> bx(vector.begin(), vector.end());

    for (uint64_t r_row_i = 0; r_row_i < result.size(); r_row_i++) {
        uint64_t r_column_counter = 0;
        uint64_t a_column_i = 0;
        uint64_t b_column_i = 0;
        float res_sum = 0.0F;

        while (a_column_i < width && b_column_i < bx.size()) {
            if (indeces[r_row_i * width + a_column_i] == b_column_i) {
                res_sum += values[r_row_i * width + a_column_i] * bx[b_column_i];
                a_column_i++;
                b_column_i++;
            } else if (indeces[r_row_i * width + a_column_i] > b_column_i) {
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
            result.resize(r_row_i + 1);
            break;
        }

        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(uint64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }
    float* res_values = new float[result.size() * max_width];
    uint64_t* res_indeces = new uint64_t[result.size() * max_width];
    
    if (res_values && res_indeces) {
        for (uint64_t x_row_i = 0; x_row_i < result.size(); x_row_i++) {
            memcpy(res_values + x_row_i * max_width, r_values[x_row_i], r_row_lengths[x_row_i] * sizeof(float));
            memcpy(res_indeces + x_row_i * max_width, r_indices[x_row_i], r_row_lengths[x_row_i] * sizeof(uint64_t));
        }
    }

       for (auto i: result) {
        std::cout << "result " << i << ' ';
    }
    for (uint64_t x_xow_i = 0; x_xow_i < result.size(); x_xow_i++) {
        delete[] r_values[x_xow_i];
        delete[] r_indices[x_xow_i];
    }
    delete[] r_values;
    delete[] r_indices;

    delete[] r_row_values;
    delete[] r_row_indices;
   

 
        


    // Clean up allocated memory
    delete[] values;
    delete[] indeces;

    return 0;
}

