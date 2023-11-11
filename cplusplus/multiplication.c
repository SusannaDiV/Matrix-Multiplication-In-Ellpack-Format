#include "multiplication.h"
#include <string.h>
#include "ellpack_utility.h"
#include <inttypes.h>

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
    struct EllpackMatrix *by = (struct EllpackMatrix *) b;
    struct EllpackMatrix *bx = transpose_ellpack((struct EllpackMatrix *) b);
        printf("trasposta width: %" PRId64 "\n", bx->width);
        printf("trasposta height: %" PRId64 "\n", bx->height);


    for (u_int64_t r_row_i = 0; r_row_i < r->height; r_row_i++) { // Zeileniteration
        
        
        u_int64_t r_column_counter = 0; // only not null results are written to result row, to mantain ellpack form
        for (u_int64_t b_row_i = 0; b_row_i < bx->height; b_row_i++) {
            // indices of currently merged entries, iterate through the row of a and b
            u_int64_t a_column_i = 0;
            u_int64_t b_column_i = 0;
            float res_sum = 0.0F;
            // check if merging ended
            while (a_column_i < ax->width && b_column_i < bx->width) {
                // the indices are equal and are to be multiplied, otherwise the lower one increments
                if (ax->indices[r_row_i * ax->width + a_column_i] == bx->indices[b_row_i * bx->width + b_column_i]) {
                    res_sum += ax->values[r_row_i * ax->width + a_column_i] * bx->values[b_row_i * bx->width + b_column_i];
                    a_column_i++;
                    b_column_i++;
                } else if (ax->indices[r_row_i * ax->width + a_column_i] > bx->indices[b_row_i * bx->width + b_column_i]) {
                    b_column_i++;
                } else {
                    a_column_i++;
                }
            }
            // only add the result entry if it s not zero
            if (res_sum != 0.0) {
                r_row_values[r_column_counter] = res_sum;
                r_row_indices[r_column_counter] = b_row_i;
                r_column_counter++;
            }
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
