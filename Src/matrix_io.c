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

    if (mm_is_complex(matcode) && mm_is_matrix(matcode) &&
            mm_is_sparse(matcode)) {
        fprintf(stderr, "This application does not support complex matrices.\n");
        exit(1);
    }

    if (mm_read_mtx_crd_size(f, &mat->M, &mat->N, &mat->nz) != 0) {
        fprintf(stderr, "Error reading matrix size.\n");
        exit(1);
    }

    // Alloca COO
    mat->I = (int*)malloc(mat->nz * sizeof(int));
    mat->J = (int*)malloc(mat->nz * sizeof(int));
    mat->val = (double*)malloc(mat->nz * sizeof(double));

    // Leggi COO
    for (int i = 0; i < mat->nz; i++) {
        fscanf(f, "%d %d %lg\n", &mat->I[i], &mat->J[i], &mat->val[i]);
        mat->I[i]--;  // Converti da 1-based a 0-based
        mat->J[i]--;
    }

    fclose(f);
    
    printf("Matrix loaded: %d x %d, nnz: %d\n", mat->M, mat->N, mat->nz);
    
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

