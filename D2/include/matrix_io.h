#ifndef MATRIX_IO_H
#define MATRIX_IO_H

typedef struct {
    int M;              // righe
    int N;              // colonne
    int nz;             // non-zero
    int is_symmetric;
    int *I, *J;         // coordinate COO
    double *val;        // valori
    int *prefixSum;     // CSR prefix
    int *sorted_J;      // CSR colonne ordinate
    double *sorted_val; // CSR valori ordinati
} Matrix;

Matrix* read_matrix(const char *filename);
void free_matrix(Matrix *mat);

#endif
