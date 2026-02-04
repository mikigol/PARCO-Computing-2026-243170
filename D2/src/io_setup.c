#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"
#include "matrix_io.h" 

void convert_coo_to_csr(int *I, int *J, double *V, int nz, int rows, LocalCSR *dest);

void load_and_scatter_matrix(const char *filename, int rank, int size, 
                             LocalCSR *local_mat, int *M_glob, int *N_glob, int *nz_glob) {
    
    if (rank == 0) {
        printf("Rank 0: Reading matrix %s...\n", filename);
        Matrix *mat = read_matrix(filename);
        if (!mat) {
            fprintf(stderr, "Error reading matrix\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        *M_glob = mat->M;
        *N_glob = mat->N;
        *nz_glob = mat->nz;

        MPI_Bcast(M_glob, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(N_glob, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int *counts = (int*)calloc(size, sizeof(int));
        for (int i = 0; i < mat->nz; i++) {
            counts[GET_OWNER(mat->I[i], size)]++;
        }

        for (int p = 1; p < size; p++) {
            int p_nz = counts[p];
            MPI_Send(&p_nz, 1, MPI_INT, p, 0, MPI_COMM_WORLD);

            int *buf_I = malloc(p_nz * sizeof(int));
            int *buf_J = malloc(p_nz * sizeof(int));
            double *buf_V = malloc(p_nz * sizeof(double));
            
            int curr = 0;
            for(int k=0; k < mat->nz; k++) {
                if (GET_OWNER(mat->I[k], size) == p) {
                    buf_I[curr] = mat->I[k];
                    buf_J[curr] = mat->J[k];
                    buf_V[curr] = mat->val[k];
                    curr++;
                }
            }
            MPI_Send(buf_I, p_nz, MPI_INT, p, 1, MPI_COMM_WORLD);
            MPI_Send(buf_J, p_nz, MPI_INT, p, 2, MPI_COMM_WORLD);
            MPI_Send(buf_V, p_nz, MPI_DOUBLE, p, 3, MPI_COMM_WORLD);
            
            free(buf_I); free(buf_J); free(buf_V);
        }

        int my_nz = counts[0];
        int *my_I = malloc(my_nz * sizeof(int));
        int *my_J = malloc(my_nz * sizeof(int));
        double *my_V = malloc(my_nz * sizeof(double));
        
        int k = 0;
        for (int i = 0; i < mat->nz; i++) {
            if (GET_OWNER(mat->I[i], size) == 0) {
                my_I[k] = mat->I[i];
                my_J[k] = mat->J[i];
                my_V[k] = mat->val[i];
                k++;
            }
        }
        free(counts);
        free_matrix(mat);

        int my_rows = (*M_glob + size - 1 - rank) / size;
        for(int i=0; i<my_nz; i++) my_I[i] = GET_LOCAL_IDX(my_I[i], size);
        
        convert_coo_to_csr(my_I, my_J, my_V, my_nz, my_rows, local_mat);
        free(my_I); free(my_J); free(my_V);

    } else {

        MPI_Bcast(M_glob, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(N_glob, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int my_nz;
        MPI_Recv(&my_nz, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *l_I = malloc(my_nz * sizeof(int));
        int *l_J = malloc(my_nz * sizeof(int));
        double *l_V = malloc(my_nz * sizeof(double));

        MPI_Recv(l_I, my_nz, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(l_J, my_nz, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(l_V, my_nz, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int my_rows = (*M_glob + size - 1 - rank) / size;
        if ((*M_glob % size != 0) && (rank >= *M_glob % size)) my_rows = *M_glob/size;
        else my_rows = *M_glob/size + 1;

        for(int i=0; i<my_nz; i++) l_I[i] = GET_LOCAL_IDX(l_I[i], size);

        convert_coo_to_csr(l_I, l_J, l_V, my_nz, my_rows, local_mat);
        free(l_I); free(l_J); free(l_V);
    }
}

void convert_coo_to_csr(int *I, int *J, double *V, int nz, int rows, LocalCSR *dest) {
    dest->n_local_rows = rows;
    dest->n_local_nz = nz;
    dest->row_ptr = calloc(rows + 1, sizeof(int));
    
    for (int i = 0; i < nz; i++) dest->row_ptr[I[i] + 1]++;
    
    for (int i = 0; i < rows; i++) dest->row_ptr[i+1] += dest->row_ptr[i];
    
    dest->col_ind = malloc(nz * sizeof(int));
    dest->val = malloc(nz * sizeof(double));
    
    int *temp_ptr = malloc(rows * sizeof(int));
    memcpy(temp_ptr, dest->row_ptr, rows * sizeof(int));
    
    for (int i = 0; i < nz; i++) {
        int row = I[i];
        int idx = temp_ptr[row]++;
        dest->col_ind[idx] = J[i];
        dest->val[idx] = V[i];
    }
    free(temp_ptr);
}
