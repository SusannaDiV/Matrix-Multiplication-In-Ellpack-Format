#include "ellpack_utility.h"


// Partly inspired from https://stackoverflow.com/a/41129764
struct EllpackMatrix* make_ellpack(u_int64_t real_width, u_int64_t height, u_int64_t width, char* file) {
    struct EllpackMatrix* ellpack = malloc(sizeof(struct EllpackMatrix));
    ellpack->real_width = real_width;
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

void free_all(struct EllpackMatrix *matrices[], int n) {
    for (int i = 0; i < n; i++) {
        free_ellpack(matrices[i]);
    }
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
    
    if (!walker || !r_values || !r_indices || !r_row_lengths || !r_row_indices || !r_row_values) {
        r->height = 0; // skip all loops and go to cleanup
    }
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
