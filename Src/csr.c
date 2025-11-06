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

void csr_spmv_parallel(Matrix *mat, double *x, double *y, int num_threads) {
    #pragma omp parallel for num_threads(num_threads) schedule(runtime)
    for(int i = 0; i < mat->M; i++) {
        for(int k = mat->prefixSum[i]; k < mat->prefixSum[i + 1]; k++) {
            y[i] += mat->sorted_val[k] * x[mat->sorted_J[k]];
        }
    }
}


