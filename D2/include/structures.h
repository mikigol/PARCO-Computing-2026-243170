#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <mpi.h>

#define GET_OWNER(glob_idx, size) ((glob_idx) % (size))
#define GET_LOCAL_IDX(glob_idx, size) ((glob_idx) / (size))

typedef struct {
    int n_local_rows;
    int n_local_nz;
    int *row_ptr;
    int *col_ind;
    double *val;
} LocalCSR;

typedef struct {
    int num_ghosts;
    int total_to_send;
    
    double *send_buffer;
    double *recv_buffer;
    
    int *send_counts;
    int *recv_counts;
    int *sdispls;
    int *rdispls;
    
    int *export_indices;     
} CommInfo;

void free_local_csr(LocalCSR *mat);
void free_comm_info(CommInfo *comm);

#endif
