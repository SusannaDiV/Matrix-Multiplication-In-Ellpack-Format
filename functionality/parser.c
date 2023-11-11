#include "parser.h"
#include "ellpack_utility.h"

#include <error.h>
#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

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
