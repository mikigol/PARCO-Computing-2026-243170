
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
    // TEST PARALLEL - TUTTE LE COMBINAZIONI
    // ============================================
    int chunk_sizes[] = {10, 100, 1000};
    char* schedule_names[] = {"static", "dynamic", "guided"};
    
    printf("\n\n╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  CSR PARALLEL - SCHEDULE COMPARISON (%d threads)           ║\n", num_threads);
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");
    
    for (int s = 0; s < 3; s++) {
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        printf("Schedule: %s\n", schedule_names[s]);
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        
        for (int c = 0; c < 3; c++) {
            double times_par[NRUNS];
            
            printf("  Chunk Size: %d\n", chunk_sizes[c]);
            
            // 10 runate per questa combinazione schedule/chunk
            for(int run = 0; run < NRUNS; run++) {
                double start, stop;
                double total_time = 0.0;
                double dummy_2 = 0.0;
                
                printf("    Run %d/%d...\n", run + 1, NRUNS);
                
                for(int iter = 0; iter < ITER; iter++) {
                    memset(y, 0, mat->M * sizeof(double));
                    
                    GET_TIME(start);
                    csr_spmv_parallel_schedule(mat, x, y, num_threads, s, chunk_sizes[c]);
                    GET_TIME(stop);
                    
                    total_time += stop - start;
                    for(int i = 0; i < mat->M; i++) {
                        dummy_2 += y[i];
                    }
                }
                
                times_par[run] = total_time / ITER;
            }
            
        }
    }
    free(x);
    free(y);
    free_matrix(mat);

    return 0;
}


