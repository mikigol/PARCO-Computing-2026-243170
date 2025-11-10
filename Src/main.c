#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "matrix_io.h"
#include "csr.h"
#include "my_timer.h"

#define NRUNS 10

int compare_double(const void *a, const void *b) {
    double diff = (*(double*)a - *(double*)b);
    return (diff > 0) - (diff < 0);
}

double calculate_percentile_90(double *times, int n) {
    qsort(times, n, sizeof(double), compare_double);
    double index = 0.90 * (n - 1);
    int lower = (int)floor(index);
    int upper = (int)ceil(index);

    if (lower == upper) return times[lower];

    double weight = index - lower;
    return times[lower] * (1 - weight) + times[upper] * weight;
}

int main(int argc, char *argv[]) {
    // Modalità bash: args = <matrix> <threads> <schedule> <chunk_size>
    // Modalità sequenziale: args = <matrix> 1 none none

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

    // Determina se è sequenziale o parallelo
    int is_sequential = (strcmp(schedule_str, "none") == 0);

    int schedule = 0;  // default static
    int chunk_size = 1;

    if (!is_sequential) {
        // Parsing schedule
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

    // Carica matrice
    Matrix *mat = read_matrix(matrix_file);
    coo_to_csr(mat);

    double *x = (double*)calloc(mat->M, sizeof(double));
    double *y = (double*)calloc(mat->M, sizeof(double));

    for(int i = 0; i < mat->M; i++) {
        x[i] = 1.0;
    }

double times[NRUNS];
double dummy = 0.0;

if (is_sequential) {
    for(int run = 0; run < NRUNS; run++) {
        double start, stop;
        memset(y, 0, mat->M * sizeof(double));
        
        GET_TIME(start);
        csr_spmv_seq(mat, x, y);
        GET_TIME(stop);
        
        times[run] = stop - start;
        for(int i = 0; i < mat->M; i++) dummy += y[i];
    }
} else {
    for(int run = 0; run < NRUNS; run++) {
        double start, stop;
        
        memset(y, 0, mat->M * sizeof(double));
        
        GET_TIME(start);
        csr_spmv_parallel_schedule(mat, x, y, num_threads, schedule, chunk_size);
        GET_TIME(stop);
        
        times[run] = stop - start;
        for(int i = 0; i < mat->M; i++) dummy += y[i];
    }
}


    // Calcola 90% percentile
    double p90 = calculate_percentile_90(times, NRUNS);

    // OUTPUT PER BASH SCRIPT: stampa SOLO il tempo (in secondi)
    // Lo script bash si aspetta solo un numero
    printf("%.6f\n", p90);

    // Scrivi dettagli su stderr per debug (opzionale, non interferisce con bash)
    fprintf(stderr, "[DEBUG] P90: %.6f sec (%.4f ms), Dummy: %.6e\n",
            p90, p90 * 1000, dummy);

    free(x);
    free(y);
    free_matrix(mat);

    return 0;
}
