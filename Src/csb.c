//
//  csb.c
//  Deliverable
//
//  Created by Mikele Golemi on 04/11/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "csb.h"

CSB_Matrix* csr_to_csb(Matrix *csr_mat) {
    CSB_Matrix *csb = (CSB_Matrix*)malloc(sizeof(CSB_Matrix));
    
    csb->M = csr_mat->M;
    csb->nnz = csr_mat->nz;
    csb->beta = (int)sqrt((double)csr_mat->M);
    if(csb->beta == 0) csb->beta = 1;
    
    csb->num_blockrows = (csr_mat->M + csb->beta - 1) / csb->beta;
    csb->num_blockcols = (csr_mat->M + csb->beta - 1) / csb->beta;
    
    printf("\n===== CSB CONVERSION =====\n");
    printf("Beta: %d, Blocks: %d x %d = %d\n",
           csb->beta, csb->num_blockrows, csb->num_blockcols,
           csb->num_blockrows * csb->num_blockcols);
    
    // Alloca
    csb->rowind = (int*)malloc(csb->nnz * sizeof(int));
    csb->colind = (int*)malloc(csb->nnz * sizeof(int));
    csb->val = (double*)malloc(csb->nnz * sizeof(double));
    csb->blkptr = (int*)malloc((csb->num_blockrows * csb->num_blockcols + 1) * sizeof(int));
    
    int *block_counts = (int*)calloc(csb->num_blockrows * csb->num_blockcols, sizeof(int));
    
    // FASE 1: Conteggio
    printf("Phase 1: Counting...\n");
    for(int i = 0; i < csr_mat->M; i++) {
        for(int k = csr_mat->prefixSum[i]; k < csr_mat->prefixSum[i + 1]; k++) {
            int j = csr_mat->sorted_J[k];
            int bi = i / csb->beta;
            int bj = j / csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            block_counts[block_id]++;
        }
    }
    
    // FASE 2: Puntatori
    printf("Phase 2: Computing pointers...\n");
    csb->blkptr[0] = 0;
    for(int b = 0; b < csb->num_blockrows * csb->num_blockcols; b++) {
        csb->blkptr[b + 1] = csb->blkptr[b] + block_counts[b];
        block_counts[b] = csb->blkptr[b];
    }
    
    // FASE 3: Inserimento
    printf("Phase 3: Reordering...\n");
    for(int i = 0; i < csr_mat->M; i++) {
        for(int k = csr_mat->prefixSum[i]; k < csr_mat->prefixSum[i + 1]; k++) {
            int j = csr_mat->sorted_J[k];
            double v = csr_mat->sorted_val[k];
            
            int bi = i / csb->beta;
            int bj = j / csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            
            int local_i = i % csb->beta;
            int local_j = j % csb->beta;
            
            int csb_idx = block_counts[block_id]++;
            csb->val[csb_idx] = v;
            csb->rowind[csb_idx] = local_i;
            csb->colind[csb_idx] = local_j;
        }
    }
    
    free(block_counts);
    printf("CSB conversion complete!\n==========================\n\n");
    
    return csb;
}

void csb_spmv_seq(CSB_Matrix *csb, double *x, double *y) {
    for(int bi = 0; bi < csb->num_blockrows; bi++) {
        int row_start = bi * csb->beta;
        
        for(int bj = 0; bj < csb->num_blockcols; bj++) {
            int col_start = bj * csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            int block_start = csb->blkptr[block_id];
            int block_end = csb->blkptr[block_id + 1];
            
            for(int k = block_start; k < block_end; k++) {
                int i = row_start + csb->rowind[k];
                int j = col_start + csb->colind[k];
                y[i] += csb->val[k] * x[j];
            }
        }
    }
}

void csb_spmv_parallel(CSB_Matrix *csb, double *x, double *y, int num_threads) {
   #pragma omp parallel for num_threads(num_threads) schedule(runtime)
    for(int bi = 0; bi < csb->num_blockrows; bi++) {
        int row_start = bi * csb->beta;
        
        // Vettore PRIVATO per questa blockrow
        double *y_temp = (double*)calloc(csb->beta, sizeof(double));
        
        // Ciclo seriale: ogni thread elabora la sua blockrow
        for(int bj = 0; bj < csb->num_blockcols; bj++) {
            int col_start = bj * csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            int block_start = csb->blkptr[block_id];
            int block_end = csb->blkptr[block_id + 1];
            
            for(int k = block_start; k < block_end; k++) {
                y_temp[csb->rowind[k]] += csb->val[k] * x[col_start + csb->colind[k]];
            }
        }
        
       
        for(int i = 0; i < csb->beta; i++) {
            y[row_start + i] += y_temp[i];
        }
        
        free(y_temp);
    }
}

void free_csb(CSB_Matrix *csb) {
    if(csb) {
        free(csb->rowind);
        free(csb->colind);
        free(csb->val);
        free(csb->blkptr);
        free(csb);
    }
}


