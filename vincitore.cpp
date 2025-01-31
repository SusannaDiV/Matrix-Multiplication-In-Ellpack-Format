#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <error.h>
#include <stdbool.h>

/** the struct storing the representation matrices and dimensions */
struct EllpackMatrix {
    u_int64_t real_width;
    u_int64_t height;
    u_int64_t width;
    float *values;
    u_int64_t *indices;
};

// Partly inspired from https://stackoverflow.com/a/41129764
struct EllpackMatrix* make_ellpack(u_int64_t real_width, u_int64_t height, u_int64_t width, char* file) {
    struct EllpackMatrix* ellpack = malloc(sizeof(struct EllpackMatrix));
    ellpa#include <iostream>
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

void free_ellpack(EllpackMatrix* x) {
    delete[] x->values;
    delete[] x->indices;
    delete x;
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
    line_count = 4;

    while (std::getline(matrix_file, matrix_line)) {
        std::istringstream token_stream(matrix_line);
        std::string token;
        std::getline(token_stream, token, ';');
        errno = 0;
        long row_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t row = static_cast<uint64_t>(row_raw);

        std::getline(token_stream, token, ';');
        errno = 0;
        long column_raw = std::strtol(token.c_str(), const_cast<char**>(&end_ptr), 10);
        uint64_t column = static_cast<uint64_t>(column_raw);

        std::getline(token_stream, token, ';');
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
                out_file << run_row << ';' << real_column << ';' << std::scientific << value_entry;
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

    write_matrix(result, "out1.mat");

    std::cout << "[FREE] Freeing used memory ...\n";

    return 0;
}
ck->real_width = real_width;
    ellpack->width = width;
    ellpack->height = height;
    ellpack->values = malloc(sizeof(float) * width * height);
    ellpack->indices = malloc(sizeof(u_int64_t) * width * height);
    return ellpack;
}

void free_ellpack(struct EllpackMatrix *x) {
    free(x -> values);
    free(x -> indices);
    free(x);
}


u_int64_t realwidth_ellpack(const struct EllpackMatrix * const x) {

    // go through each row to the last right index and find the largest of them
    u_int64_t max_width = 0;
    for (u_int64_t i = 0; i < x->height; i++) {
        u_int64_t j = 0;
        while (j < x -> width && x->values[i * x -> width + j] != 0.0) {
            j++;
        }
        if (x -> indices[i * x->width + --j] > max_width) {
            max_width = x->indices[i * x -> width + j];
        }
    }
    return max_width + 1; // to include column n the width needs to be at least n+1
}

void flatten_ellpack(struct EllpackMatrix *x, float **values, u_int64_t **indices, u_int64_t *lengths) {
    x->values = calloc(x->height * x->width, sizeof(float));
    x->indices = calloc(x->height * x->width, sizeof(u_int64_t));
    if (x->values && x->indices) {
        for (u_int64_t x_row_i = 0; x_row_i < x->height; x_row_i++) {
            memcpy(x->values + x_row_i * x->width, values[x_row_i], lengths[x_row_i] * sizeof(float));
            memcpy(x->indices + x_row_i * x->width, indices[x_row_i], lengths[x_row_i] * sizeof(u_int64_t));
        }
    }
    for (u_int64_t x_xow_i = 0; x_xow_i < x->height; x_xow_i++) {
        free(values[x_xow_i]);
        free(indices[x_xow_i]);
    }
    free(values);
    free(indices);
    free(lengths);
}

struct EllpackMatrix *transpose_ellpack(const struct EllpackMatrix * x) {

    // create r as x transposed in ellpack directly
    // for each row of r search the corresponding column indices in x
    // store only the row numbers where the current transposed column was found
    struct EllpackMatrix *r = malloc(sizeof(*r));
    r->height = realwidth_ellpack(x);
    r->width = 1;
    float *r_row_values = calloc (x->height, sizeof(float));
    u_int64_t *r_row_indices = calloc (x->height, sizeof(u_int64_t));


    u_int64_t *walker = calloc(x->height, sizeof(u_int64_t)); // an index of the last read position in each row of x
    // temporary arrays to store each new row of r with the least size
    float **r_values = calloc(r->height, sizeof(float *));
    u_int64_t **r_indices = calloc (r->height, sizeof(u_int64_t *));
    u_int64_t *r_row_lengths = calloc(r->height, sizeof(u_int64_t));
    u_int64_t max_width = 0;
    // temporary row storage of max size before its size is known
    
    for (u_int64_t r_row_i = 0; r_row_i < r->height; r_row_i++) {
        u_int64_t r_column_c = 0; // how many rows in x contain the currently transposed column
        for (u_int64_t x_row_i = 0; x_row_i < x->height; x_row_i++) {
            // the next read position in this row of x contains the number of the current row of r (column of x)
            if (walker[x_row_i] < x->width && x->indices[x_row_i * x->width + walker[x_row_i]] == r_row_i) {
                r_row_values[r_column_c] = x->values[x_row_i * x->width + walker[x_row_i]];
                r_row_indices[r_column_c] = x_row_i; // add the x row as a column to r row
                r_column_c++;
                walker[x_row_i]++;
            }
        }
        if (r_column_c > max_width) { // keep track of the least width needed to store the entire matrix
            max_width = r_column_c;
        }
        r_values[r_row_i] = calloc(r_column_c, sizeof(float));
        r_indices[r_row_i] = calloc(r_column_c, sizeof(u_int64_t));
        if (!r_values[r_row_i] || !r_indices[r_row_i]) {
            r->height = r_row_i + 1; // only clean up to this row in the flatten method
            break;
        }
        memcpy(r_values[r_row_i], r_row_values, r_column_c * sizeof(float));
        memcpy(r_indices[r_row_i], r_row_indices, r_column_c * sizeof(u_int64_t));
        r_row_lengths[r_row_i] = r_column_c;
    }
    // now create the 2d arrays with the least required width and store the result there
    r->width = max_width;
    flatten_ellpack(r, r_values, r_indices, r_row_lengths);

    free(walker);
    free(r_row_values);
    free(r_row_indices);
    return r;
}

void matr_mult_ellpack(const void* a, const void* b, void* result) {
    struct EllpackMatrix *r = (struct EllpackMatrix *) result;
    struct EllpackMatrix *ax = (struct EllpackMatrix *) a;
    r->height = ax->height;
    float **r_values = calloc(ax->height, sizeof(float *));
    u_int64_t **r_indices = calloc(ax->height, sizeof(u_int64_t *));
    u_int64_t *r_row_lengths = calloc(ax->height, sizeof(u_int64_t));
    float *r_row_values = calloc(ax->height, sizeof(float));
    u_int64_t *r_row_indices = calloc(ax->height, sizeof(u_int64_t));
    u_int64_t max_width = 0;


        // array of upper limit size for each result row
    if (!r_values || !r_indices || !r_row_lengths) {
        r->height = 0; // skip all loops and go to cleanup
    }
    struct EllpackMatrix *bx = transpose_ellpack((struct EllpackMatrix *) b);
    for (u_int64_t r_row_i = 0; r_row_i < r->height; r_row_i++) { // Zeileniteration
        u_int64_t r_column_counter = 0; // only not null results are written to result row, to mantain ellpack form
            // indices of currently merged entries, iterate through the row of a and b
            u_int64_t a_column_i = 0;
            u_int64_t b_column_i = 0;
            float res_sum = 0.0F;
            // check if merging ended
            while (a_column_i < ax->width && b_column_i < bx->width) {
                // the indices are equal and are to be multiplied, otherwise the lower one increments
                if (ax->indices[r_row_i * ax->width + a_column_i] == bx->indices[b_column_i]) {
                    res_sum += ax->values[r_row_i * ax->width + a_column_i] * bx->values[b_column_i];
                    a_column_i++;
                    b_column_i++;
                } else if (ax->indices[r_row_i * ax->width + a_column_i] > bx->indices[b_column_i]) {
                    b_column_i++;
                } else {
                    a_column_i++;
                }
            }
            // only add the result entry if it s not zero
            if (res_sum != 0.0) {
                r_row_values[r_column_counter] = res_sum;
                r_column_counter++;
            }
        
        if (r_column_counter > max_width) {
            max_width = r_column_counter;
        }
        // store the resulting row with its length
        r_values[r_row_i] = calloc(r_column_counter, sizeof(float));
        r_indices[r_row_i] = calloc(r_column_counter, sizeof(u_int64_t));
        if (!r_values[r_row_i] || !r_indices[r_row_i]) {
            r->height = r_row_i + 1; // only clean up to this row in the flatten method
            break;
        }
        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(u_int64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }
    r->width = max_width;
    flatten_ellpack(r, r_values, r_indices, r_row_lengths);
    free(r_row_values);
    free(r_row_indices);
    free_ellpack(bx);
}




















#define CSV_LINE_LENGTH 61
static int skip_lines(FILE *file, unsigned long long num) {
    char line[20] = {0};
    for(unsigned long long run = 0; run < num; ++run) {
        if (!fgets(line, 20, file)) {
            return -1;
        }
    }
    return 0;
}

struct EllpackMatrix* parse_matrix(char *matrix_path) {
    FILE *matrix_file = fopen(matrix_path, "r");
    // Read the first 2 lines containing WIDTH\nHEIGHT in u_int64_t format
    char line[20] = {0};
    unsigned long long line_count = 0;
    char* end_ptr;
    long width = 0;
    long height = 0;

    while (fgets(line, 20, matrix_file)) {
        if(line_count > 1) {
            break;
        } else if (line_count == 0) {
            errno = 0;
            height = strtol(line, &end_ptr, 10);
        } else if (line_count == 1) {
            errno = 0;
            width = strtol(line, &end_ptr, 10);
        }
        ++line_count;
    }
    // Scan matrix for shrinking possibilities
    // CSV separator is ; -> FLOAT;INT;INT so line length = (float) 18 + (int) 20 + (int) 20 + (;) 2 + (\n) 1 = 61
    printf("[SCAN] Scanning matrix %s ...\n", matrix_path);

    char matrix_line[CSV_LINE_LENGTH] = {0};
    char* token;

    line_count = 4;

    u_int64_t max_width = 0;
    u_int64_t current_row = 0;
    u_int64_t count_used = 0;

    while (fgets(matrix_line, CSV_LINE_LENGTH, matrix_file)) {
        token = strtok(matrix_line, ";");
        
        errno = 0;
        long row_raw = strtol(token, &end_ptr, 10);
        u_int64_t row = (u_int64_t) row_raw;
       if(row > current_row) {
            current_row = row;
            if(max_width < count_used) {
                max_width = count_used;
            }
            count_used = 0;
        }

        token = strtok(NULL, ";");
    
    
        errno = 0;
        long column_raw = strtol(token, &end_ptr, 10);
        u_int64_t column = (u_int64_t) column_raw;
        ++count_used;
        ++line_count;
    }

    if(max_width <= 0) {
        max_width = (u_int64_t) width;
    }

    printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", width, max_width);
    printf("[INIT] Allocating %lu bytes of memory for matrix %s\n", (4 * max_width * height) + (8 * max_width * height), matrix_path);

    struct EllpackMatrix* matrix = make_ellpack((u_int64_t) width, (u_int64_t) height, max_width, matrix_path);

    for(u_int64_t run_row = 0; run_row < matrix->height; ++run_row) {
        for(u_int64_t run_col = 0; run_col < matrix->width; ++run_col) {
            matrix->indices[run_row * matrix->width + run_col] = 0;
            matrix->values[run_row * matrix->width + run_col] = 0;
        }
    }

    printf("[INIT] Reading data of matrix %s into memory\n", matrix_path);

    // Go to start of file (after height and width declarations)
    rewind(matrix_file);

    int skr = skip_lines(matrix_file, 3);
    // Read matrix in CSV format
    // CSV separator is ; -> FLOAT;INT;INT so line length = (float) 18 + (int) 20 + (int) 20 + (;) 2 + (\n) 1 = 61

    line_count = 4;

    while (fgets(matrix_line, CSV_LINE_LENGTH, matrix_file)) {
        token = strtok(matrix_line, ";");
        errno = 0;
        long row_raw = strtol(token, &end_ptr, 10);
        u_int64_t row = (u_int64_t) row_raw;

        token = strtok(NULL, ";");
        errno = 0;
        long column_raw = strtol(token, &end_ptr, 10);
        u_int64_t column = (u_int64_t) column_raw;

        token = strtok(NULL, ";");
        errno = 0;
        float value = strtof(token, &end_ptr);

        u_int64_t new_col = 0;
        bool col_found = false;

        for (u_int64_t r_col = 0; r_col < matrix->width; ++r_col) {
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

    fclose(matrix_file);
    return matrix;
}

//inutile per me, basta ritornare la struttura
void write_matrix(struct EllpackMatrix* matrix, char *out_path) {
    FILE *out_file = fopen(out_path, "w");
    // Print the matrix in reduced coordinate schema
    for(u_int64_t run_row = 0; run_row < matrix->height; ++run_row) {
        for(u_int64_t run_col = 0; run_col < matrix->width; ++run_col) {
            u_int64_t index_entry = matrix->indices[run_row * matrix->width + run_col];
            float value_entry = matrix->values[run_row * matrix->width + run_col];
            if(value_entry != 0) {
                u_int64_t real_column = index_entry;
                fprintf(out_file, "%lu;%lu;%.9e", run_row, real_column, value_entry);
                if(run_row < (matrix->height - 1) || run_col < (matrix->width - 1)) {
                    fprintf(out_file, "\n");
                }
            }
        }
    }

    fclose(out_file);
}

int main (int argc, char** argv) {
    struct EllpackMatrix* amatrix = parse_matrix("a.mat");
    struct EllpackMatrix* bmatrix = parse_matrix("b.mat");

    struct EllpackMatrix* result = calloc(1, sizeof(*result));
    matr_mult_ellpack(amatrix, bmatrix, result);

    write_matrix(result, "out.mat");
    printf("[FREE] Freeing used memory ...\n");
    free_ellpack(amatrix);
    free_ellpack(bmatrix);
    free_ellpack(result);
}
