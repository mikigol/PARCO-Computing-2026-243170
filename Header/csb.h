//
//  csb.h
//  Deliverable
//
//  Created by Mikele Golemi on 04/11/25.
//
#ifndef CSB_H
#define CSB_H

#include "matrix_io.h"

typedef struct {
    int *rowind;
    int *colind;
    double *val;
    int *blkptr;
    int M;
    int beta;
    int nnz;
    int num_blockrows;
    int num_blockcols;
} CSB_Matrix;

// Conversione CSR a CSB
CSB_Matrix* csr_to_csb(Matrix *csr_mat);

// Moltiplicazione sequenziale
void csb_spmv_seq(CSB_Matrix *csb, double *x, double *y);

// Moltiplicazione parallela
void csb_spmv_parallel(CSB_Matrix *csb, double *x, double *y, int num_threads);

// Libera memoria
void free_csb(CSB_Matrix *csb);

#endif


