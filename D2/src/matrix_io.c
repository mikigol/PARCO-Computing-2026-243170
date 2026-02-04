#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix_io.h"
#include "mmio.h"


Matrix* read_matrix(const char *filename) {
    Matrix *mat = (Matrix*)malloc(sizeof(Matrix));
       
       MM_typecode matcode;
       FILE *f = fopen(filename, "r");
       if (!f) {
           fprintf(stderr, "Error opening file: %s\n", filename);
           exit(1);
       }

       if (mm_read_banner(f, &matcode) != 0) {
           fprintf(stderr, "Could not process Matrix Market banner.\n");
           exit(1);
       }

       printf("Matrix type: %s\n", mm_typecode_to_str(matcode));
       printf("  Real: %s\n", mm_is_real(matcode) ? "yes" : "no");
       printf("  Complex: %s\n", mm_is_complex(matcode) ? "yes" : "no");
       printf("  Symmetric: %s\n", mm_is_symmetric(matcode) ? "yes" : "no");
       printf("  Pattern: %s\n", mm_is_pattern(matcode) ? "yes" : "no");

       if (!mm_is_matrix(matcode) || !mm_is_sparse(matcode)) {
           fprintf(stderr, "Error: only supports matrix, sparse format.\n");
           exit(1);
       }

       if (mm_read_mtx_crd_size(f, &mat->M, &mat->N, &mat->nz) != 0) {
           fprintf(stderr, "Error reading matrix size.\n");
           exit(1);
       }

       printf("Matrix size: %d x %d, NNZ (file): %d\n", mat->M, mat->N, mat->nz);

       int max_nz = mm_is_symmetric(matcode) ? (2 * mat->nz) : mat->nz;
       mat->I = (int*)malloc(max_nz * sizeof(int));
       mat->J = (int*)malloc(max_nz * sizeof(int));
       mat->val = (double*)malloc(max_nz * sizeof(double));

       int nz_actual = 0;
       
       for (int i = 0; i < mat->nz; i++) {
           int row, col;
           double value = 1.0;  
           
           fscanf(f, "%d %d", &row, &col);
           
           if (!mm_is_pattern(matcode)) {
               fscanf(f, "%lf", &value);
           }
           
           row--;
           col--;
           
           mat->I[nz_actual] = row;
           mat->J[nz_actual] = col;
           mat->val[nz_actual] = value;
           nz_actual++;
           
           if (mm_is_symmetric(matcode) && row != col) {
               mat->I[nz_actual] = col;
               mat->J[nz_actual] = row;
               mat->val[nz_actual] = value;
               nz_actual++;
           }
       }

       fclose(f);
       
      
       mat->nz = nz_actual;
       mat->is_symmetric = mm_is_symmetric(matcode);
       
       printf("Actual NNZ (after symmetry expansion): %d\n\n", mat->nz);
       
       return mat;
}

void free_matrix(Matrix *mat) {
    if (mat) {
        free(mat->I);
        free(mat->J);
        free(mat->val);
        free(mat);
    }
}
