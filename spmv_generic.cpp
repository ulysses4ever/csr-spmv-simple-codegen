#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


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
    double mytime;
    char c;

    // Read CSR matrix from file
    FILE *csr_file = fopen(csr_filename, "r");
    if (csr_file == NULL) {
        perror("Error opening csr_file");
        exit(EXIT_FAILURE);
    }

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
    FILE *x_file = fopen(vector_filename, "r");
    if (x_file == NULL) {
        perror("Error opening x_file");
        exit(EXIT_FAILURE);
    }

    while (x_size < cols && fscanf(x_file, "%lf,", &x[x_size]) == 1) {
        x_size++;
    }
    fclose(x_file);
    // done reading x vector

    // technically, if we don't do this in the loop, we'll ccompute garbage,
    // but that's good enough for benchmarking purposes
    memset(y, 0, sizeof(double)*rows);

    ankerl::nanobench::Bench bench;
    std::vector<ankerl::nanobench::Result> results;

    bench.output(nullptr).run("spmv", [&] {
        for (int i = 0; i < rows; i++) {
            int row_start = indptr[i];
            int row_end = indptr[i + 1];
            for (int j = row_start; j < row_end; j++) {
                y[i] += csr_val[j] * x[indices[j]];
            }
        }
    });

    printf("Time: %.2f ns\n", bench.results()[0].median(ankerl::nanobench::Result::Measure::elapsed) * 1e9);

    // Use result vector y to avoid the compiler optimizing away the computation
    double sum = 0;
    for (int i=0; i<rows; i++) {
        sum += y[i];
    }
    printf("%.2f\n", sum);
}
