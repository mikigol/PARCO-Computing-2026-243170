#ifdef _OPENMP
    #include <omp.h>
#else
    #define omp_get_thread_num() 0
    #define omp_get_num_threads() 1
#endif
#include "structures.h"

void compute_spmv(LocalCSR *mat, double *x, double *y) {
    
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < mat->n_local_rows; i++) {
        double sum = 0.0;
        int start = mat->row_ptr[i];
        int end = mat->row_ptr[i+1];

        for (int j = start; j < end; j++) {
            sum += mat->val[j] * x[mat->col_ind[j]];
        }
        y[i] = sum;
    }
}
