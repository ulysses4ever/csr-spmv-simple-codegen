#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Comparison function for doubles
int compare_doubles(const void *a, const void *b) {
    double arg1 = *(const double *)a;
    double arg2 = *(const double *)b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void spmv_sparse(
        double *restrict y,
        const double *restrict csr_val,
        const int *restrict indices,
        const int *restrict indptr,
        const double *restrict x,
        const int rpntr_size) {
    for (int i = 0; i < rpntr_size; i++) {
        int row_start = indptr[i];
        int row_end = indptr[i + 1];
        for (int j = row_start; j < row_end; j++) {
            y[i] += csr_val[j] * x[indices[j]];
        }
    }
}

const int ITERS = 100;

int main(int argc, char *argv[]) {

    // Parse command line arguments
    if (argc != 6) { // expecting 5 arguments (+ the program name)
        fprintf(stderr, "Usage: %s <rows> <cols> <nnz> <csr_filename> <vector_filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);
    int nnz = atoi(argv[3]);
    char *csr_filename = argv[4];
    char *vector_filename = argv[5];

    double *y = (double*)malloc(rows * sizeof(double));
    double *x = (double*)malloc(cols * sizeof(double));
    double *csr_val = (double*)malloc(nnz * sizeof(double));
    int *indices = (int*)malloc(nnz * sizeof(int));
    int *indptr = (int*)malloc((rows + 1) * sizeof(int));
    struct timespec t1, t2;
    double times[ITERS];

    for (int i=0; i<ITERS; i++) {
        FILE *csr_file = fopen(csr_filename, "r");
        if (csr_file == NULL) {
            perror("Error opening csr_file");
            exit(EXIT_FAILURE);
        }
        FILE *x_file = fopen(vector_filename, "r");
        if (x_file == NULL) {
            perror("Error opening x_file");
            exit(EXIT_FAILURE);
        }

        memset(y, 0, sizeof(double)*rows);
        char c;

        // Read CSR matrix from file
        int x_size=0, val_size=0;
        assert(fscanf(csr_file, "indptr=[%c", &c) == 1);
        if (c != ']') {
            ungetc(c, csr_file);
            assert(fscanf(csr_file, "%d", &indptr[val_size]) == 1);
            val_size++;
            while (1) {
                assert(fscanf(csr_file, "%c", &c) == 1);
                if (c == ',') {
                    assert(fscanf(csr_file, "%d", &indptr[val_size]) == 1);
                    val_size++;
                } else if (c == ']') {
                    break;
                } else {
                    assert(0);
                }
            }
        }
        assert(fscanf(csr_file, "%c", &c) == 1 && c == '\n');
        val_size=0;
        assert(fscanf(csr_file, "indices=[%d", &indices[val_size]) == 1.0);
        val_size++;
        while (1) {
            assert(fscanf(csr_file, "%c", &c) == 1);
            if (c == ',') {
                assert(fscanf(csr_file, "%d", &indices[val_size]) == 1.0);
                val_size++;
            } else if (c == ']') {
                break;
            } else {
                assert(0);
            }
        }
        if(fscanf(csr_file, "%c", &c));
        assert(c=='\n');
        val_size=0;
        assert(fscanf(csr_file, "data=[%lf", &csr_val[val_size]) == 1.0);
        val_size++;
        while (1) {
            assert(fscanf(csr_file, "%c", &c) == 1);
            if (c == ',') {
                assert(fscanf(csr_file, "%lf", &csr_val[val_size]) == 1.0);
                val_size++;
            } else if (c == ']') {
                break;
            } else {
                assert(0);
            }
        }
        fclose(csr_file);
        // done reading csr matrix

        // Read x vector from file
        while (x_size < cols && fscanf(x_file, "%lf,", &x[x_size]) == 1) {
            x_size++;
        }
        fclose(x_file);

        clock_gettime(CLOCK_MONOTONIC, &t1);
        spmv_sparse(y, csr_val, indices, indptr, x, rows);
        clock_gettime(CLOCK_MONOTONIC, &t2);

        times[i] = (t2.tv_sec - t1.tv_sec) * 1e9 + (t2.tv_nsec - t1.tv_nsec);
    }

    qsort(times, ITERS, sizeof(double), compare_doubles);
    printf("Time: %.2f ms\n", times[ITERS/2]);

    // Print result vector y to avoid the compiler optimizing away the computation
    for (int i=0; i<rows; i++) {
        printf("%.2f\n", y[i]);
    }
}
