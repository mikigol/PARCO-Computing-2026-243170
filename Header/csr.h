

#ifndef CSR_H
#define CSR_H

#include "matrix_io.h"

void csr_spmv_seq(Matrix *mat, double *x, double *y);

void csr_spmv_parallel_schedule(Matrix *mat, double *x, double *y, 
                                  int num_threads, int schedule_type, int chunk_size);

#endif


