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

    // ============================================
    // TEST CSR SEQUENZIALE
    // ============================================
    printf("\n===== CSR SEQUENTIAL =====\n");
    double start, stop;
    double total_time_csr_seq = 0.0;
    double dummy = 0.0;
    
    memset(y, 0, mat->M * sizeof(double));
    
    for(int iter = 0; iter < ITER; iter++) {
        
        
        GET_TIME(start);
        csr_spmv_seq(mat, x, y);
        GET_TIME(stop);
        
        total_time_csr_seq += stop - start;
        
        for(int i = 0; i < mat->M; i++) {
            dummy += y[i];
        }
    }

    printf("Dummy: %f\n", dummy);
    printf("Average time: %f sec\n", total_time_csr_seq / ITER);
    printf("==========================\n");

    // ============================================
    // TEST CSR PARALLELO
    // ============================================
    printf("\n===== CSR  PARALLEL (%d threads) =====\n", num_threads);
    double total_time_csr_par = 0.0;
    dummy = 0.0;
    memset(y, 0, mat->M * sizeof(double));
    for(int iter = 0; iter < ITER; iter++) {
        
        
        GET_TIME(start);
        csr_spmv_parallel(mat, x, y, 8);
        GET_TIME(stop);
        
        total_time_csr_par += stop - start;
        
        for(int i = 0; i < mat->M; i++) {
            dummy += y[i];
        }
    }

    printf("Dummy: %f\n", dummy);
    printf("Average time: %f sec\n", total_time_csr_par / ITER);
    printf("Speedup: %fx\n", total_time_csr_seq / total_time_csr_par);
    printf("====================================\n");

    // ============================================
    // TEST CSB SEQUENZIALE
    // ============================================
    printf("\n===== CSB SEQUENTIAL =====\n");
    double total_time_csb_seq = 0.0;
    dummy = 0.0;
    memset(y, 0, mat->M * sizeof(double));
    for(int iter = 0; iter < ITER; iter++) {
        
        
        GET_TIME(start);
        csb_spmv_seq(csb, x, y);
        GET_TIME(stop);
        
        total_time_csb_seq += stop - start;
        
        for(int i = 0; i < mat->M; i++) {
            dummy += y[i];
        }
    }

    printf("Dummy: %f\n", dummy);
    printf("Average time: %f sec\n", total_time_csb_seq / ITER);
    printf("==========================\n");

    // ============================================
    // TEST CSB PARALLELO
    // ============================================
    printf("\n===== CSB PARALLEL (%d threads) =====\n", num_threads);
    double total_time_csb_par = 0.0;
    dummy = 0.0;
    memset(y, 0, mat->M * sizeof(double));
    for(int iter = 0; iter < ITER; iter++) {
        
        
        GET_TIME(start);
        csb_spmv_parallel(csb, x, y, 8);
        GET_TIME(stop);
        
        total_time_csb_par += stop - start;
        
        for(int i = 0; i < mat->M; i++) {
            dummy += y[i];
        }
    }

    printf("Dummy: %f\n", dummy);
    printf("Average time: %f sec\n", total_time_csb_par / ITER);
    printf("Speedup: %fx\n", total_time_csb_seq / total_time_csb_par);
    printf("====================================\n");

    // ============================================
    // CONFRONTO FINALE
    // ============================================
    printf("\n===== COMPARISON SUMMARY =====\n");
    printf("CSR Sequential:  %f sec\n", total_time_csr_seq / ITER);
    printf("CSR Parallel:    %f sec (speedup: %.2fx)\n",
           total_time_csr_par / ITER, total_time_csr_seq / total_time_csr_par);
    printf("CSB Sequential:  %f sec\n", total_time_csb_seq / ITER);
    printf("CSB Parallel:    %f sec (speedup: %.2fx)\n",
           total_time_csb_par / ITER, total_time_csb_seq / total_time_csb_par);
    printf("\nCSB vs CSR (sequential): %.2fx\n",
           total_time_csr_seq / total_time_csb_seq);
    printf("CSB vs CSR (parallel):   %.2fx\n",
           total_time_csr_par / total_time_csb_par);
    printf("==============================\n");

    // Cleanup
    free(x);
    free(y);
    free_csb(csb);
    free_matrix(mat);

    return 0;
}


