#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "matrix_io.h"
#include "csr.h"
#include "my_timer.h"

#define ITER_NEVER_PERF 10
#define ITER_PERF 1

int compare_doubles(const void *a, const void *b) {
    double diff = (*(double*)a - *(double*)b);
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}

double calculate_90th_percentile(double *times, int n) {
    if (n == 1) return times[0];
    qsort(times, n, sizeof(double), compare_doubles);
    double k = 0.90 * n;
    int index = (int)k;
    if (k == (double)index && index > 0)
        return (times[index-1] + times[index]) / 2.0;
    if (k != (double)index)
        index = (int)(k + 1);
    return times[index];
}

int main(int argc, char *argv[]) {
    if(argc < 5) {
        fprintf(stderr, "Usage: %s <matrix.mtx> <num_threads> <schedule> <chunk_size>\n", argv[0]);
        fprintf(stderr, "  For sequential: %s <matrix.mtx> 1 none none\n", argv[0]);
        fprintf(stderr, "  For parallel: %s <matrix.mtx> <threads> <static|dynamic|guided> <chunk>\n", argv[0]);
        return 1;
    }

    char *matrix_file = argv[1];
    int num_threads = atoi(argv[2]);
    char *schedule_str = argv[3];
    char *chunk_str = argv[4];

    int is_sequential = (strcmp(schedule_str, "none") == 0);

    int schedule = 0;
    int chunk_size = 1;

    if (!is_sequential) {
        if (strcmp(schedule_str, "static") == 0) schedule = 0;
        else if (strcmp(schedule_str, "dynamic") == 0) schedule = 1;
        else if (strcmp(schedule_str, "guided") == 0) schedule = 2;
        else {
            fprintf(stderr, "Error: invalid schedule '%s'\n", schedule_str);
            return 1;
        }

        chunk_size = atoi(chunk_str);
        if (chunk_size <= 0) {
            fprintf(stderr, "Error: chunk_size must be > 0\n");
            return 1;
        }
    }

    if(num_threads <= 0) {
        fprintf(stderr, "Error: num_threads must be > 0\n");
        return 1;
    }

    Matrix *mat = read_matrix(matrix_file);
    coo_to_csr(mat);

    double *x = (double*)calloc(mat->M, sizeof(double));
    double *y = (double*)calloc(mat->M, sizeof(double));

    for(int i = 0; i < mat->M; i++) {
        x[i] = 1.0;
    }

    int iterations = 0;
#ifdef PERF_MODE
    iterations = ITER_PERF;
#else
    iterations = ITER_NEVER_PERF;
#endif

    double *times = (double*)malloc(iterations * sizeof(double));
    double dummy = 0.0;

    for(int iter = 0; iter < iterations; iter++) {
        memset(y, 0, mat->M * sizeof(double));
        double start, stop;

        if (is_sequential) {
            GET_TIME(start);
            csr_spmv_seq(mat, x, y);
            GET_TIME(stop);
        } else {
            GET_TIME(start);
            csr_spmv_parallel_schedule(mat, x, y, num_threads, schedule, chunk_size);
            GET_TIME(stop);
        }
        times[iter] = stop - start;

        for(int i = 0; i < mat->M; i++) {
            dummy += y[i];
        }
    }

#ifndef PERF_MODE
    double p90 = calculate_90th_percentile(times, iterations);
    printf("%.6f\n", p90);
    fprintf(stderr, "[DEBUG] Iter: %d, 90th percentile time: %.6f sec (%.4f ms), Dummy: %.6e\n",
            iterations, p90, p90 * 1000, dummy);
#else
    printf("%.6f\n", times[0]);
    fprintf(stderr, "[DEBUG PERF_MODE] Iter: %d, Dummy: %.6e\n", iterations, dummy);
#endif

    free(times);
    free(x);
    free(y);
    free_matrix(mat);

    return 0;
}

