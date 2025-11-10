//
//  csr.c
//  Deliverable
//
//  Created by Mikele Golemi on 04/11/25.
//



#include <omp.h>

#include "csr.h"

void csr_spmv_seq(Matrix *mat, double *x, double *y) {
    for(int i = 0; i < mat->M; i++) {
        for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
            y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
        }
    }
}

void csr_spmv_parallel_schedule(Matrix *mat, double *x, double *y, 
                                  int num_threads, int schedule_type, int chunk_size) {
    switch (schedule_type) {
        case 0:  // static
            #pragma omp parallel for num_threads(num_threads) schedule(static, chunk_size)
            for(int i = 0; i < mat->M; i++) {
                for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
                    y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
                }
            }
            break;
        case 1:  // dynamic
            #pragma omp parallel for num_threads(num_threads) schedule(dynamic, chunk_size)
            for(int i = 0; i < mat->M; i++) {
                for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
                    y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
                }
            }
            break;
        case 2:  // guided
            #pragma omp parallel for num_threads(num_threads) schedule(guided, chunk_size)
            for(int i = 0; i < mat->M; i++) {
                for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
                    y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
                }
            }
            break;
    }
}




