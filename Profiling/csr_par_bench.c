#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "matrix_io.h"
#include "csr.h"
#include "my_timer.h"

#define ITER 10000
#define NRUNS 5

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <matrix.mtx> [num_threads] [schedule] [chunk_size]\n", argv[0]);
        fprintf(stderr, "Default: 4 threads, static schedule, chunk 1000\n");
        return 1;
    }
    
    int num_threads = 4;
    int schedule_type = 0;
    int chunk_size = 1000;
    
    if(argc >= 3) num_threads = atoi(argv[2]);
    if(argc >= 4) schedule_type = atoi(argv[3]);
    if(argc >= 5) chunk_size = atoi(argv[4]);

    Matrix *mat = read_matrix(argv[1]);
    coo_to_csr(mat);

    double *x = (double*)calloc(mat->M, sizeof(double));
    double *y = (double*)calloc(mat->M, sizeof(double));
    
    for(int i = 0; i < mat->M; i++) {
        x[i] = 1.0;
    }

    
    char* schedule_names[] = {"static", "dynamic", "guided"};

    printf("\n╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  CSR PARALLEL - PERF BENCHMARK                               ║\n");
    printf("║  Matrix: %d x %d, NNZ: %d\n", mat->M, mat->M, mat->nz);
    printf("║  Threads: %d, Schedule: %s, Chunk: %d\n", 
           num_threads, schedule_names[schedule_type], chunk_size);
    printf("║  %d runs x %d iterations\n", NRUNS, ITER);
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");
    
    for(int run = 0; run < NRUNS; run++) {
        double dummy= 0.0;
        printf("  Run %d/%d...\n", run + 1, NRUNS);
        
        for(int iter = 0; iter < ITER; iter++) {
            memset(y, 0, mat->M * sizeof(double));
            
            switch (schedule_type) {
                case 0:
                    #pragma omp parallel for num_threads(num_threads) schedule(static, chunk_size)
                    for(int i = 0; i < mat->M; i++) {
                        for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
                            y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
                        }
                    }
                    break;
                case 1:
                    #pragma omp parallel for num_threads(num_threads) schedule(dynamic, chunk_size)
                    for(int i = 0; i < mat->M; i++) {
                        for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
                            y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
                        }
                    }
                    break;
                case 2:
                    #pragma omp parallel for num_threads(num_threads) schedule(guided, chunk_size)
                    for(int i = 0; i < mat->M; i++) {
                        for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
                            y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
                        }
                    }
                    break;
            }
            
            for(int i = 0; i < mat->M; i++) {
                dummy_global += y[i];
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
