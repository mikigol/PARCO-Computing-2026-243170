//
//  matrix_io.h
//  Deliverable
//
//  Created by Mikele Golemi on 04/11/25.
//

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

// Legge matrice da file MTX
Matrix* read_matrix(const char *filename);

// Converte COO a CSR
void coo_to_csr(Matrix *mat);

// Libera memoria
void free_matrix(Matrix *mat);

#endif


