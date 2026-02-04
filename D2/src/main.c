#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#ifdef _OPENMP
    #include <omp.h>
#else
    #define omp_get_thread_num() 0
    #define omp_get_num_threads() 1
#endif
#include "structures.h"

void load_and_scatter_matrix(const char *f, int r, int s, LocalCSR *m, int *Mg, int *Ng, int *nz);
void setup_communication_pattern(LocalCSR *m, CommInfo *c, int r, int s, int Ng);
void perform_ghost_exchange(CommInfo *c, double *x, int dim);
void compute_spmv(LocalCSR *m, double *x, double *y);

void generate_synthetic_matrix(int rows_per_proc, int nnz_per_row, int rank, int size, LocalCSR *local_mat, int *M_glob, int *N_glob, int *nz_glob);

int compare_doubles(const void *a, const void *b) {
    double arg1 = *(const double *)a;
    double arg2 = *(const double *)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    int provided, rank, size;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            printf("Usage Strong: %s <matrix.mtx> [repeats]\n", argv[0]);
            printf("Usage Weak:   %s synthetic <repeats> <rows_per_proc> <nnz_per_row>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    char *arg1 = argv[1];
    int is_synthetic = (strcmp(arg1, "synthetic") == 0);
    int repeats = 10; 

    LocalCSR local_mat = {0};
    int M_glob, N_glob, nz_glob;

    if (is_synthetic) {

        if (argc < 5) {
            if (rank == 0) printf("Error: Synthetic mode requires: synthetic <repeats> <rows_per_proc> <nnz_per_row>\n");
            MPI_Finalize(); 
            return 1;
        }
        repeats = atoi(argv[2]);
        int rows_pp = atoi(argv[3]);
        int nnz_pp = atoi(argv[4]);
        
        generate_synthetic_matrix(rows_pp, nnz_pp, rank, size, &local_mat, &M_glob, &N_glob, &nz_glob);
        
    } else {
        if (argc > 2) repeats = atoi(argv[2]);
        load_and_scatter_matrix(arg1, rank, size, &local_mat, &M_glob, &N_glob, &nz_glob);
    }
    
    CommInfo comm = {0};
    setup_communication_pattern(&local_mat, &comm, rank, size, N_glob);

    
    int my_x_dim = 0;
   
    int chunk = (N_glob + size - 1) / size; 
    int start_col = rank * chunk;
    int end_col = (rank + 1) * chunk;
    if (end_col > N_glob) end_col = N_glob;
    my_x_dim = end_col - start_col;
    if (my_x_dim < 0) my_x_dim = 0;
    
    double *full_x = malloc((my_x_dim + comm.num_ghosts) * sizeof(double));
    double *local_y = malloc(local_mat.n_local_rows * sizeof(double));
    
    srand(rank * 1234); 
    for(int i=0; i<my_x_dim; i++) full_x[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0; 

    double *run_total_times = (double*)malloc(repeats * sizeof(double));
    double *run_comm_times  = (double*)malloc(repeats * sizeof(double));

    perform_ghost_exchange(&comm, full_x, my_x_dim);
    compute_spmv(&local_mat, full_x, local_y);
    
    MPI_Barrier(MPI_COMM_WORLD);

    for(int r=0; r<repeats; r++) {
        MPI_Barrier(MPI_COMM_WORLD);
        
        double t_start = MPI_Wtime();
        
        perform_ghost_exchange(&comm, full_x, my_x_dim);
        double t_after_comm = MPI_Wtime();
        
        compute_spmv(&local_mat, full_x, local_y);
        double t_end = MPI_Wtime();
        
        run_comm_times[r] = t_after_comm - t_after_comm; 
        run_comm_times[r] = t_after_comm - t_start;
        run_total_times[r] = t_end - t_start;
    }
    
    
    char display_name[64];
    if (is_synthetic) snprintf(display_name, 64, "synthetic_np%d", size);
    else strncpy(display_name, arg1, 64);

    for (int p=0; p<size; p++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == p) {
            if (p==0 && rank==0) {
                // Header CSV
                printf("matrix_name,rank,num_procs,run,elapsed_time,comm_time,local_nz,ghost_entries,local_flops\n");
            }
            long long my_flops = 2LL * local_mat.n_local_nz;
            for(int r=0; r<repeats; r++) {
                printf("%s,%d,%d,%d,%.9f,%.9f,%d,%d,%lld\n", 
                       display_name, rank, size, r, run_total_times[r], run_comm_times[r], 
                       local_mat.n_local_nz, comm.num_ghosts, my_flops);
            }
        }
    }

   
    double *sorted_times = malloc(repeats * sizeof(double));
    for(int i=0; i<repeats; i++) sorted_times[i] = run_total_times[i];
    qsort(sorted_times, repeats, sizeof(double), compare_doubles);
    
    int p90_idx = (int)(repeats * 0.90);
    if(p90_idx >= repeats) p90_idx = repeats - 1;
    double my_p90 = sorted_times[p90_idx];
    
    free(sorted_times); free(run_total_times); free(run_comm_times);

    double global_max_p90 = 0.0;
    MPI_Reduce(&my_p90, &global_max_p90, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    long long my_nz = local_mat.n_local_nz;
    long long total_flops_sym = 0;
    long long my_flops_calc = 2LL * my_nz;
    MPI_Reduce(&my_flops_calc, &total_flops_sym, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    long long my_ghosts = comm.num_ghosts;
    long long min_g, max_g, sum_g, max_nz, sum_nz;
    MPI_Reduce(&my_ghosts, &min_g, 1, MPI_LONG_LONG, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&my_ghosts, &max_g, 1, MPI_LONG_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&my_ghosts, &sum_g, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&my_nz, &max_nz, 1, MPI_LONG_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&my_nz, &sum_nz, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double avg_g = (double)sum_g / size;
        double avg_nz = (double)sum_nz / size;
        double imb_ratio = (avg_nz > 0) ? (double)max_nz / avg_nz : 0.0;
        double gflops = (double)total_flops_sym / global_max_p90 / 1e9;

        printf("\n\n=== SUMMARY METRICS TABLE ===\n");
        printf("Matrix_Name,Num_Processes,Ghost_Entries_Min,Ghost_Entries_Avg,Ghost_Entries_Max,Load_Imbalance_Ratio,System_P90_Time,Total_GFLOPs\n");
        printf("%s,%d,%lld,%.2f,%lld,%.4f,%.9f,%.4f\n", 
               display_name, size, min_g, avg_g, max_g, imb_ratio, global_max_p90, gflops);
        printf("=============================\n");
    }

    free(local_mat.val);
    free(local_mat.col_ind);
    free(local_mat.row_ptr);
    free(full_x);
    free(local_y);
    
    MPI_Finalize();
    return 0;
}


