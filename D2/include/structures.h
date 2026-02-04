#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <mpi.h>

// Macro per distribuzione ciclica (usate ovunque)
#define GET_OWNER(glob_idx, size) ((glob_idx) % (size))
#define GET_LOCAL_IDX(glob_idx, size) ((glob_idx) / (size))

// Struttura per la matrice CSR locale
typedef struct {
    int n_local_rows;
    int n_local_nz;
    int *row_ptr;
    int *col_ind;
    double *val;
} LocalCSR;

// Struttura per gestire la comunicazione (Ghost exchange)
typedef struct {
    int num_ghosts;
    int total_to_send;
    
    // Buffer
    double *send_buffer;
    double *recv_buffer;
    
    // MPI Arrays per Alltoallv
    int *send_counts;
    int *recv_counts;
    int *sdispls;
    int *rdispls;
    
    // Indici per mappare i dati
    int *export_indices;     // Quali miei elementi devo copiare nel buffer di invio
} CommInfo;

void free_local_csr(LocalCSR *mat);
void free_comm_info(CommInfo *comm);

#endif
