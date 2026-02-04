#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifdef _OPENMP
    #include <omp.h>
#else
   
    #define omp_get_thread_num() 0
    #define omp_get_num_threads() 1
#endif
#include "structures.h"

// Prepara le strutture dati per MPI_Alltoallv
void setup_communication_pattern(LocalCSR *mat, CommInfo *comm, int rank, int size, int N_globale) {
    int *ghost_flags = calloc(N_globale, sizeof(int));
    int n_ghosts = 0;

    // 1. Trova quali colonne sono esterne (ghost)
    for (int i = 0; i < mat->n_local_nz; i++) {
        int g_col = mat->col_ind[i];
        if (GET_OWNER(g_col, size) != rank) {
            if (ghost_flags[g_col] == 0) {
                ghost_flags[g_col] = 1;
                n_ghosts++;
            }
        }
    }
    comm->num_ghosts = n_ghosts;

    // 2. Crea lista richieste
    int *requested_ghosts = malloc(n_ghosts * sizeof(int));
    int *remap_array = malloc(N_globale * sizeof(int));
    memset(remap_array, -1, N_globale * sizeof(int)); // Init a -1

    int count = 0;
    for (int c = 0; c < N_globale; c++) {
        if (ghost_flags[c]) {
            requested_ghosts[count] = c;
            remap_array[c] = count; // Mappa: Globale -> Indice nel buffer Ghost
            count++;
        }
    }
    free(ghost_flags);

    // 3. Rinumera colonne in CSR:
    // Se locale -> indice locale
    // Se ghost -> num_local_vec + indice_ghost
    int my_x_dim = 0; // Dimensione parte locale del vettore
    for(int k=0; k<N_globale; k++) if(GET_OWNER(k, size) == rank) my_x_dim++;

    for (int i = 0; i < mat->n_local_nz; i++) {
        int g_col = mat->col_ind[i];
        if (GET_OWNER(g_col, size) == rank) {
            mat->col_ind[i] = GET_LOCAL_IDX(g_col, size);
        } else {
            mat->col_ind[i] = my_x_dim + remap_array[g_col];
        }
    }
    free(remap_array);

    // 4. Scambio informazioni su chi deve inviare cosa (Handshake)
    comm->send_counts = calloc(size, sizeof(int));
    comm->recv_counts = calloc(size, sizeof(int));

    // Dico a ogni processo quanti dati voglio da lui
    for(int i=0; i<n_ghosts; i++) {
        comm->recv_counts[GET_OWNER(requested_ghosts[i], size)]++;
    }

    MPI_Alltoall(comm->recv_counts, 1, MPI_INT, comm->send_counts, 1, MPI_INT, MPI_COMM_WORLD);

    // Setup offsets
    comm->sdispls = malloc(size * sizeof(int));
    comm->rdispls = malloc(size * sizeof(int));
    comm->sdispls[0] = 0; comm->rdispls[0] = 0;
    for(int p=1; p<size; p++) {
        comm->sdispls[p] = comm->sdispls[p-1] + comm->send_counts[p-1];
        comm->rdispls[p] = comm->rdispls[p-1] + comm->recv_counts[p-1];
    }

    // Scambio effettivo degli indici richiesti
    comm->total_to_send = comm->sdispls[size-1] + comm->send_counts[size-1];
    int *indices_to_export = malloc(comm->total_to_send * sizeof(int));
    
    // Devo ordinare requested_ghosts per proprietario prima di inviare
    int *sorted_reqs = malloc(n_ghosts * sizeof(int));
    int *offsets = calloc(size, sizeof(int));
    memcpy(offsets, comm->rdispls, size * sizeof(int));

    for(int i=0; i<n_ghosts; i++) {
        int owner = GET_OWNER(requested_ghosts[i], size);
        sorted_reqs[offsets[owner]++] = requested_ghosts[i];
    }
    free(requested_ghosts); free(offsets);

    MPI_Alltoallv(sorted_reqs, comm->recv_counts, comm->rdispls, MPI_INT,
                  indices_to_export, comm->send_counts, comm->sdispls, MPI_INT, MPI_COMM_WORLD);
    
    // Preparo la lista di esportazione (Global -> Local)
    comm->export_indices = malloc(comm->total_to_send * sizeof(int));
    for(int i=0; i<comm->total_to_send; i++) {
        comm->export_indices[i] = GET_LOCAL_IDX(indices_to_export[i], size);
    }

    // Alloco buffer finali
    comm->send_buffer = malloc(comm->total_to_send * sizeof(double));
    comm->recv_buffer = malloc(n_ghosts * sizeof(double));

    free(sorted_reqs);
    free(indices_to_export);
}

// Esegue lo scambio runtime
void perform_ghost_exchange(CommInfo *comm, double *full_x, int local_dim) {
    // 1. Pack (OpenMP parallel)
    #pragma omp parallel for
    for (int i = 0; i < comm->total_to_send; i++) {
        comm->send_buffer[i] = full_x[comm->export_indices[i]];
    }

    // 2. MPI Exchange
    MPI_Alltoallv(comm->send_buffer, comm->send_counts, comm->sdispls, MPI_DOUBLE,
                  comm->recv_buffer, comm->recv_counts, comm->rdispls, MPI_DOUBLE, 
                  MPI_COMM_WORLD);

    // 3. Unpack (diretto in coda a x)
    #pragma omp parallel for
    for (int i = 0; i < comm->num_ghosts; i++) {
        full_x[local_dim + i] = comm->recv_buffer[i];
    }
}


// --- IMPLEMENTAZIONE GENERATORE SINTETICO (BLOCK DISTRIBUTION) ---
// Allineata con la logica del main (chunk) per coerenza tra Weak e Strong scaling
void generate_synthetic_matrix(int rows_per_proc, int nnz_per_row, int rank, int size, LocalCSR *local_mat, int *M_glob, int *N_glob, int *nz_glob) {
    
    int i, j;

    // 1. Calcolo dimensioni globali
    // Weak scaling: La dimensione totale cresce col numero di processi
    *M_glob = rows_per_proc * size;
    *N_glob = *M_glob; // Matrice quadrata

    // 2. Allocazione Struttura Locale
    local_mat->n_local_rows = rows_per_proc;
    
    // Stima per allocazione (con un margine di sicurezza per la varianza random)
    int estimated_nz = rows_per_proc * (nnz_per_row + 10); 
    
    local_mat->row_ptr = (int *)malloc((rows_per_proc + 1) * sizeof(int));
    local_mat->col_ind = (int *)malloc(estimated_nz * sizeof(int));
    local_mat->val = (double *)malloc(estimated_nz * sizeof(double));

    if (!local_mat->row_ptr || !local_mat->col_ind || !local_mat->val) {
        fprintf(stderr, "Allocazione memoria fallita nel rank %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // 3. Generazione Dati (Logica Random per colonne)
    srand(rank * 12345 + 789); // Seed deterministico ma diverso per rank
    
    int current_total_nz = 0;
    local_mat->row_ptr[0] = 0;

    // --- DIFFERENZA CHIAVE: Qui usiamo logica locale standard, non ciclica ---
    // Ogni processo riempie le sue 'rows_per_proc' righe.
    // L'offset globale è implicito (Rank 0: righe 0..99, Rank 1: 100..199, etc.)
    
    for (i = 0; i < rows_per_proc; i++) {
        // Variabilità realistica nel numero di elementi per riga (+/- 20%)
        int variance = nnz_per_row / 5; 
        if (variance < 1) variance = 1;
        int actual_nnz = nnz_per_row + (rand() % (2 * variance + 1)) - variance;
        
        // Clamp (non meno di 1, non più della dimensione matrice)
        if (actual_nnz < 1) actual_nnz = 1;
        if (actual_nnz > *N_glob) actual_nnz = *N_glob;

        // Generiamo colonne uniche casuali
        for (j = 0; j < actual_nnz; j++) {
            int col;
            int is_duplicate;
            
            // Tentativi per trovare una colonna non ancora usata in questa riga
            do {
                is_duplicate = 0;
                col = rand() % (*N_glob); // Colonna casuale su tutta la matrice
                
                // Controllo duplicati semplice (scansiona la riga corrente)
                for (int k = local_mat->row_ptr[i]; k < current_total_nz; k++) {
                    if (local_mat->col_ind[k] == col) {
                        is_duplicate = 1;
                        break;
                    }
                }
            } while (is_duplicate);

            // Scrittura CSR
            local_mat->col_ind[current_total_nz] = col;
            local_mat->val[current_total_nz] = ((double)rand() / RAND_MAX) * 2.0 - 1.0; // Valori tra -1 e 1
            current_total_nz++;
            
            // Controllo sicurezza array overflow (realloc se necessario)
            if (current_total_nz >= estimated_nz) {
                estimated_nz *= 2;
                local_mat->col_ind = realloc(local_mat->col_ind, estimated_nz * sizeof(int));
                local_mat->val = realloc(local_mat->val, estimated_nz * sizeof(double));
            }
        }
        local_mat->row_ptr[i + 1] = current_total_nz;
    }
    
    local_mat->n_local_nz = current_total_nz;

    // 4. Reallocazione finale alla dimensione esatta per risparmiare RAM
    local_mat->col_ind = realloc(local_mat->col_ind, current_total_nz * sizeof(int));
    local_mat->val = realloc(local_mat->val, current_total_nz * sizeof(double));

    // 5. Statistiche Globali
    long long loc_nz = local_mat->n_local_nz;
    long long glob_nz_long = 0;
    MPI_Allreduce(&loc_nz, &glob_nz_long, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
    *nz_glob = (int)glob_nz_long;

    if (rank == 0) {
        printf("--- Generated Synthetic Matrix (Block Distribution) ---\n");
        printf("Global Rows: %d, Global Cols: %d, Total NNZ: %d\n", *M_glob, *N_glob, *nz_glob);
        printf("Weak Scaling Mode: %d rows/proc, %d nnz/row (avg)\n", rows_per_proc, nnz_per_row);
        printf("-----------------------------------------------------\n");
    }
}
