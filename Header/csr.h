//
//  csr.h
//  Deliverable
//
//  Created by Mikele Golemi on 04/11/25.
//

#ifndef CSR_H
#define CSR_H

#include "matrix_io.h"

// Moltiplicazione CSR sequenziale
void csr_spmv_seq(Matrix *mat, double *x, double *y);

// Moltiplicazione CSR parallela
void csr_spmv_parallel(Matrix *mat, double *x, double *y, int num_threads);

#endif


