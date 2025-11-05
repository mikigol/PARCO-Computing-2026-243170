#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "matrix_io.h"
#include "csr.h"
#include "csb.h"
#include "my_timer.h"

#define ITER 10000



int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <matrix.mtx>\n", argv[0]);
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

    // Leggi matrice
    Matrix *mat = read_matrix(argv[1]);
    
    // Converti a CSR
    coo_to_csr(mat);

    // Converti a CSB
    CSB_Matrix *csb = csr_to_csb(mat);

    // Alloca vettori
    double *x = (double*)calloc(mat->M, sizeof(double));
    double *y = (double*)calloc(mat->M, sizeof(double));
    
    for(int i = 0; i < mat->M; i++) {
        x[i] = 1.0;
    }

    
   
    double start, stop;
    double dummy = 0.0;
    
    

  
    printf("\n===== CSB PARALLEL (%d threads) =====\n", num_threads);
    double total_time_csb_par = 0.0;
    dummy = 0.0;
    memset(y, 0, mat->M * sizeof(double));
    for(int iter = 0; iter < ITER; iter++) {
        
        
        GET_TIME(start);
        csb_spmv_parallel(csb, x, y, num_threads);
        GET_TIME(stop);
        
        total_time_csb_par += stop - start;
        
        for(int i = 0; i < mat->M; i++) {
            dummy += y[i];
        }
    }

    
    // Cleanup
    free(x);
    free(y);
    free_csb(csb);
    free_matrix(mat);

    return 0;
}



