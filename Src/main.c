#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "matrix_io.h"
#include "csr.h"
#include "my_timer.h"

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

    double dummy = 0.0;
    double start, stop, elapsed_time;

    // Azzera il vettore risultato
    memset(y, 0, mat->M * sizeof(double));

    // Esegui SpMV (sequenziale o parallelo) e misura il tempo
    if (is_sequential) {
        GET_TIME(start);
        csr_spmv_seq(mat, x, y);
        GET_TIME(stop);
    } else {
        GET_TIME(start);
        csr_spmv_parallel_schedule(mat, x, y, num_threads, schedule, chunk_size);
        GET_TIME(stop);
    }

    // Calcola tempo trascorso
    elapsed_time = stop - start;

    // Usa dummy per evitare warning
    for(int i = 0; i < mat->M; i++) {
        dummy += y[i];
    }

    // OUTPUT PER BASH SCRIPT: stampa SOLO il tempo (in secondi)
    printf("%.6f\n", elapsed_time);

    // Scrivi dettagli su stderr per debug (opzionale)
    fprintf(stderr, "[DEBUG] Time: %.6f sec (%.4f ms), Dummy: %.6e\n",
            elapsed_time, elapsed_time * 1000, dummy);

    free(x);
    free(y);
    free_matrix(mat);

    return 0;
}
