
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "matrix_io.h"
#include "csr.h"
#include "my_timer.h"

#define ITER 10000
#define NRUNS 5


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
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <matrix.mtx> [num_threads]\n", argv[0]);
        return 1;
    }
    
    int num_threads = 4;
    if(argc >= 3) {
        num_threads = atoi(argv[2]);
        if(num_threads <= 0) {
            fprintf(stderr, "Error: num_threads deve essere > 0\n");
            return 1;
        }
    }

    Matrix *mat = read_matrix(argv[1]);
    coo_to_csr(mat);

    double *x = (double*)calloc(mat->M, sizeof(double));
    double *y = (double*)calloc(mat->M, sizeof(double));
    
    for(int i = 0; i < mat->M; i++) {
        x[i] = 1.0;
    }
    // ============================================
    // TEST CSR SEQUENZIALE
    // ============================================
    printf("\n╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  CSR SEQUENTIAL (%d runs x %d iterations)                 ║\n", NRUNS, ITER);
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
   
    double dummy = 0.0;
    for(int run = 0; run < NRUNS; run++) {
        
        
        printf("  Run %d/%d...\n", run + 1, NRUNS);
        
        for(int iter = 0; iter < ITER; iter++) {
            memset(y, 0, mat->M * sizeof(double));
            
            
            csr_spmv_seq(mat, x, y);
            
            
       

            for(int i = 0; i < mat->M; i++) {
            dummy += y[i];
            }
        }
    }
    printf("\n╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║ Dummy checksum: %.6e\n", dummy);
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
    free(x);
    free(y);
    free_matrix(mat);

    return 0;
}
