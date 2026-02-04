#!/bin/bash


MY_SOURCES="../src/main.c ../src/io_setup.c ../src/computation.c ../src/communication.c ../src/matrix_io.c ../src/mmio.c"

EXEC_MPI="../results/spmv_mpi.out"
EXEC_HYBRID="../results/spmv_hybrid.out"
DATA_DIR="../data"
RESULTS_DIR="../results"

PROCESSES_PER_NODE=24
MAX_PROCESSES=128
PROCESSES=(1 2 4 8 16 32 64 128)

OMP_SCHEDULE="dynamic,64"
OMP_PROC_BIND="close"

REPEATS=10
ROWS_PER_PROC=10000
NNZ_PER_ROW=50

MPICC="mpicc"
CFLAGS_COMMON="-O3 -Wall -lm -I../include"
CFLAGS_MPI="$CFLAGS_COMMON"
CFLAGS_HYBRID="$CFLAGS_COMMON -fopenmp"

STRONG_SCALING_CSV="$RESULTS_DIR/strong_scaling_all.csv"
WEAK_SCALING_CSV="$RESULTS_DIR/weak_scaling_all.csv"
STRONG_SCALING_HYBRID_CSV="$RESULTS_DIR/strong_scaling_hybrid.csv"
WEAK_SCALING_HYBRID_CSV="$RESULTS_DIR/weak_scaling_hybrid.csv"



find_matrices() {
    if [ ! -f ../src/main.c ]; then
        echo "❌ ERRORE CRITICO: Non trovo ../src/main.c"
        echo "   Sono nella cartella: $(pwd)"
        exit 1
    fi

    MATRICES=($(ls "$DATA_DIR"/*.mtx 2>/dev/null | xargs -n 1 basename))
    if [ ${#MATRICES[@]} -eq 0 ]; then
        echo "WARNING: Nessun file .mtx in $DATA_DIR."
    else
        echo "Trovate ${#MATRICES[@]} matrici."
    fi
}

compile_code() {
    echo "------------------------------------------------"
    rm -f "$EXEC_MPI" "$EXEC_HYBRID"
    mkdir -p "$RESULTS_DIR/logs"

    echo "📂 File sorgente: $MY_SOURCES"

    # 1. Compilazione MPI Pura
    echo "🔨 Compilazione MPI Pura..."
    $MPICC $CFLAGS_MPI $MY_SOURCES -o "$EXEC_MPI"
    
    if [ $? -ne 0 ]; then
        echo "❌ Errore compilazione MPI!"
        exit 1
    fi
    echo "✅ MPI compilato."

    echo "🔨 Compilazione Hybrid..."
    $MPICC $CFLAGS_HYBRID $MY_SOURCES -o "$EXEC_HYBRID"
    
    if [ $? -ne 0 ]; then
        echo "❌ Errore compilazione Hybrid!"
        exit 1
    fi
    echo "✅ Hybrid compilato."
    echo "------------------------------------------------"
}

initialize_csv_files() {
    HEADER="matrix_name,rank,num_procs,run,elapsed_time,comm_time,local_nz,ghost_entries,local_flops"
    echo "$HEADER" > "$STRONG_SCALING_CSV"
    echo "$HEADER" > "$WEAK_SCALING_CSV"
    echo "$HEADER" > "$STRONG_SCALING_HYBRID_CSV"
    echo "$HEADER" > "$WEAK_SCALING_HYBRID_CSV"
}

parse_output() {
    grep "," | grep -v "matrix_name"
}

run_strong_scaling() {
    local matrix="$1"
    local num_procs="$2"
    local exec="$3"
    local csv_file="$4"
    local mode="$5"
    local log_file="$RESULTS_DIR/logs/strong_${mode}_${matrix%.mtx}_np${num_procs}.log"
    
    echo "   Running STRONG ($mode): NP=$num_procs"

    local threads=1
    if [ "$mode" = "HYBRID" ]; then
        threads=$((PROCESSES_PER_NODE / num_procs))
        if [ $threads -lt 1 ]; then threads=1; fi
    fi

    mpirun -np "$num_procs" \
           -genv OMP_NUM_THREADS $threads \
           -genv OMP_SCHEDULE "$OMP_SCHEDULE" \
           -genv OMP_PROC_BIND "$OMP_PROC_BIND" \
           "$exec" "$DATA_DIR/$matrix" "$REPEATS" 2>&1 | tee "$log_file" | parse_output >> "$csv_file"
}

run_weak_scaling() {
    local num_procs="$1"
    local exec="$2"
    local csv_file="$3"
    local mode="$4"
    local log_file="$RESULTS_DIR/logs/weak_${mode}_np${num_procs}.log"
    
    echo "   Running WEAK ($mode): NP=$num_procs"

    local threads=1
    if [ "$mode" = "HYBRID" ]; then
        threads=$((PROCESSES_PER_NODE / num_procs))
        if [ $threads -lt 1 ]; then threads=1; fi
    fi

    mpirun -np "$num_procs" \
           -genv OMP_NUM_THREADS $threads \
           -genv OMP_SCHEDULE "$OMP_SCHEDULE" \
           -genv OMP_PROC_BIND "$OMP_PROC_BIND" \
           "$exec" synthetic "$REPEATS" "$ROWS_PER_PROC" "$NNZ_PER_ROW" 2>&1 | tee "$log_file" | parse_output >> "$csv_file"
}



mkdir -p "$RESULTS_DIR"
find_matrices
compile_code
initialize_csv_files

if [ ${#MATRICES[@]} -gt 0 ]; then
    echo "🟦 STRONG SCALING (MPI)"
    for matrix in "${MATRICES[@]}"; do
        for np in "${PROCESSES[@]}"; do
            if [ "$np" -le "$MAX_PROCESSES" ]; then
                run_strong_scaling "$matrix" "$np" "$EXEC_MPI" "$STRONG_SCALING_CSV" "MPI"
            fi
        done
    done

    echo "🟪 STRONG SCALING (HYBRID)"
    for matrix in "${MATRICES[@]}"; do
        for np in "${PROCESSES[@]}"; do
            if [ "$np" -le "$MAX_PROCESSES" ]; then
                run_strong_scaling "$matrix" "$np" "$EXEC_HYBRID" "$STRONG_SCALING_HYBRID_CSV" "HYBRID"
            fi
        done
    done
fi

echo "🟧 WEAK SCALING (MPI)"
for np in "${PROCESSES[@]}"; do
    if [ "$np" -le "$MAX_PROCESSES" ]; then
        run_weak_scaling "$np" "$EXEC_MPI" "$WEAK_SCALING_CSV" "MPI"
    fi
done

echo "🟨 WEAK SCALING (HYBRID)"
for np in "${PROCESSES[@]}"; do
    if [ "$np" -le "$MAX_PROCESSES" ]; then
        run_weak_scaling "$np" "$EXEC_HYBRID" "$WEAK_SCALING_HYBRID_CSV" "HYBRID"
    fi
done

echo "✅ FINITO."
