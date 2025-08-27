#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
    double times[100];
    for (int i=0; i<100; i++) {
        FILE *file1 = fopen(csr_filename, "r");
        if (file1 == NULL) {
            perror("Error opening file1");
            exit(EXIT_FAILURE);
        }
        FILE *file2 = fopen(vector_filename, "r");
        if (file2 == NULL) {
            perror("Error opening file2");
            exit(EXIT_FAILURE);
        }
        memset(y, 0, sizeof(double)*rows);
        memset(x, 0, sizeof(double)*cols);
        memset(csr_val, 0, sizeof(double)*nnz);
        memset(indices, 0, sizeof(int)*nnz);
        memset(indptr, 0, sizeof(int)*(rows + 1));
        char c;
        int x_size=0, val_size=0;
        assert(fscanf(file1, "indptr=[%c", &c) == 1);
        if (c != ']') {
            ungetc(c, file1);
            assert(fscanf(file1, "%d", &indptr[val_size]) == 1);
            val_size++;
            while (1) {
                assert(fscanf(file1, "%c", &c) == 1);
                if (c == ',') {
                    assert(fscanf(file1, "%d", &indptr[val_size]) == 1);
                    val_size++;
                } else if (c == ']') {
                    break;
                } else {
                    assert(0);
                }
            }
        }
        assert(fscanf(file1, "%c", &c) == 1 && c == '\n');
        val_size=0;
        assert(fscanf(file1, "indices=[%d", &indices[val_size]) == 1.0);
        val_size++;
        while (1) {
            assert(fscanf(file1, "%c", &c) == 1);
            if (c == ',') {
                assert(fscanf(file1, "%d", &indices[val_size]) == 1.0);
                val_size++;
            } else if (c == ']') {
                break;
            } else {
                assert(0);
            }
        }
        if(fscanf(file1, "%c", &c));
        assert(c=='\n');
        val_size=0;
        assert(fscanf(file1, "data=[%lf", &csr_val[val_size]) == 1.0);
        val_size++;
        while (1) {
            assert(fscanf(file1, "%c", &c) == 1);
            if (c == ',') {
                assert(fscanf(file1, "%lf", &csr_val[val_size]) == 1.0);
                val_size++;
            } else if (c == ']') {
                break;
            } else {
                assert(0);
            }
        }
        fclose(file1);
        while (x_size < cols && fscanf(file2, "%lf,", &x[x_size]) == 1) {
            x_size++;
        }
        fclose(file2);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        spmv_sparse(y, csr_val, indices, indptr, x, rows);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        times[i] = (t2.tv_sec - t1.tv_sec) * 1e9 + (t2.tv_nsec - t1.tv_nsec);
    }
    printf("Time: %.2f ms\\n", times[50]);
    for (int i=0; i<rows; i++) {
        printf("%.2f\\n", y[i]);
    }
    free(y);
    free(x);
    free(csr_val);
    free(indptr);
    free(indices);
}
