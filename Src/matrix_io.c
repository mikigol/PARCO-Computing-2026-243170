//
//  matrix_io.c
//  Deliverable
//
//  Created by Mikele Golemi on 04/11/25.
//

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

       // ===== STAMPA INFORMAZIONI MATRICE =====
       printf("Matrix type: %s\n", mm_typecode_to_str(matcode));
       printf("  Real: %s\n", mm_is_real(matcode) ? "yes" : "no");
       printf("  Complex: %s\n", mm_is_complex(matcode) ? "yes" : "no");
       printf("  Symmetric: %s\n", mm_is_symmetric(matcode) ? "yes" : "no");
       printf("  Pattern: %s\n", mm_is_pattern(matcode) ? "yes" : "no");

       // ===== VALIDAZIONE FORMATO =====
       if (!mm_is_matrix(matcode) || !mm_is_sparse(matcode)) {
           fprintf(stderr, "Error: only supports matrix, sparse format.\n");
           exit(1);
       }

       // ===== LEGGI DIMENSIONI =====
       if (mm_read_mtx_crd_size(f, &mat->M, &mat->N, &mat->nz) != 0) {
           fprintf(stderr, "Error reading matrix size.\n");
           exit(1);
       }

       printf("Matrix size: %d x %d, NNZ (file): %d\n", mat->M, mat->N, mat->nz);

       // ===== ALLOCA CON MARGINE PER SIMMETRIA =====
       int max_nz = mm_is_symmetric(matcode) ? (2 * mat->nz) : mat->nz;
       mat->I = (int*)malloc(max_nz * sizeof(int));
       mat->J = (int*)malloc(max_nz * sizeof(int));
       mat->val = (double*)malloc(max_nz * sizeof(double));

       // ===== LEGGI ELEMENTI CON GESTIONE SIMMETRIA E PATTERN =====
       int nz_actual = 0;
       
       for (int i = 0; i < mat->nz; i++) {
           int row, col;
           double value = 1.0;  // Default per pattern
           
           // Leggi riga e colonna
           fscanf(f, "%d %d", &row, &col);
           
           // Se NON è pattern, leggi il valore
           if (!mm_is_pattern(matcode)) {
               fscanf(f, "%lf", &value);
           }
           
           // Converti da 1-based a 0-based
           row--;
           col--;
           
           // Aggiungi elemento (row, col)
           mat->I[nz_actual] = row;
           mat->J[nz_actual] = col;
           mat->val[nz_actual] = value;
           nz_actual++;
           
           // Se simmetrica e OFF-diagonale, aggiungi simmetrico (col, row)
           if (mm_is_symmetric(matcode) && row != col) {
               mat->I[nz_actual] = col;
               mat->J[nz_actual] = row;
               mat->val[nz_actual] = value;
               nz_actual++;
           }
       }

       fclose(f);
       
       // ===== AGGIORNA NNZ FINALE =====
       mat->nz = nz_actual;
       mat->is_symmetric = mm_is_symmetric(matcode);
       
       printf("Actual NNZ (after symmetry expansion): %d\n\n", mat->nz);
       
       return mat;
}

void coo_to_csr(Matrix *mat) {
    printf("\nConverting COO to CSR...\n");
    
    // Conta elementi per riga
    int *row_counts = (int*)calloc(mat->M, sizeof(int));
    for (int i = 0; i < mat->nz; i++) {
        row_counts[mat->I[i]]++;
    }

    // Calcola prefix sum
    mat->prefixSum = (int*)malloc((mat->M + 1) * sizeof(int));
    mat->prefixSum[0] = 0;
    for (int i = 0; i < mat->M; i++) {
        mat->prefixSum[i + 1] = mat->prefixSum[i] + row_counts[i];
    }

    // Alloca array ordinati
    mat->sorted_J = (int*)malloc(mat->nz * sizeof(int));
    mat->sorted_val = (double*)malloc(mat->nz * sizeof(double));

    // Riempi array ordinati
    int *next_pos = (int*)malloc((mat->M + 1) * sizeof(int));
    for(int i = 0; i < mat->M + 1; i++) {
        next_pos[i] = mat->prefixSum[i];
    }

    for (int i = 0; i < mat->nz; i++) {
        int row = mat->I[i];
        int dest = next_pos[row];
        
        mat->sorted_val[dest] = mat->val[i];
        mat->sorted_J[dest] = mat->J[i];
        next_pos[row]++;
    }

    free(row_counts);
    free(next_pos);
    
    printf("CSR conversion complete!\n");
}

void free_matrix(Matrix *mat) {
    if (mat) {
        free(mat->I);
        free(mat->J);
        free(mat->val);
        free(mat->prefixSum);
        free(mat->sorted_J);
        free(mat->sorted_val);
        free(mat);
    }
}

