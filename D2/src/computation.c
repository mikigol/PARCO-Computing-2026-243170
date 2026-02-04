// Sostituisci #include <omp.h> con questo:
#ifdef _OPENMP
    #include <omp.h>
#else
    // Definizioni dummy per compilare senza OpenMP
    #define omp_get_thread_num() 0
    #define omp_get_num_threads() 1
#endif
#include "structures.h"

void compute_spmv(LocalCSR *mat, double *x, double *y) {
    
    // Bonus: OpenMP Scheduling guidato per bilanciare il carico
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < mat->n_local_rows; i++) {
        double sum = 0.0;
        int start = mat->row_ptr[i];
        int end = mat->row_ptr[i+1];

        // Loop vettorizzabile dal compilatore
        for (int j = start; j < end; j++) {
            sum += mat->val[j] * x[mat->col_ind[j]];
        }
        y[i] = sum;
    }
}
