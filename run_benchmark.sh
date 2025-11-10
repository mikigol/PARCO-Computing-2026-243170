#!/bin/bash

# ==========================
# CONFIGURAZIONE
# ==========================
SRC="Src/main.c"              # Sorgente dentro Src/
EXEC="./matvec"               # Eseguibile nella root
OUTPUT="results.csv"
MATRIX_DIR="Matrix"           # Directory delle matrici

# Flag di compilazione come da Makefile
CC="gcc"
CFLAGS_BASE="-Wall -g -fopenmp -std=c99 -IHeader"
LIBS="-lm"

# Matrici da testare (dentro Matrix/)
MATRICES=("bcsstk16.mtx","bcsstk18.mtx","bcsst mtx k25..mtx")
OPT_FLAGS=("" "-O1" "-O2" "-O3" )
SCHEDULES=("static" "dynamic" "guided")
CHUNKSIZES=(1 10 100 1000)
THREADS=(1 2 4 8 16 32 64)

# ==========================
# INIZIALIZZAZIONE CSV
# ==========================
echo "matrix,mode,opt_level,schedule,chunk_size,num_threads,perf_mode,metric_type,result" > "$OUTPUT"

# ==========================
# FUNZIONE: Esegui e registra
# ==========================
run_test() {
    local matrix="$1"
    local mode="$2"
    local opt="$3"
    local schedule="$4"
    local chunk="$5"
    local threads="$6"
    local use_perf="$7"

    # Percorso completo matrice
    local matrix_path="${MATRIX_DIR}/${matrix}"
    local result=""

    if [ "$use_perf" = "yes" ]; then
        # Con perf: estrai SOLO i cache miss L1
        result=$(perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
                 ${EXEC} "$matrix_path" "$threads" "$schedule" "$chunk" 2>&1 | \
                 grep "L1-dcache-load-misses" | awk '{print $1}' | sed 's/,//g')
        perf_mode="yes"
        metric_type="L1_misses"
    else
        # Senza perf: estrai il TEMPO (90% percentile)
        result=$(${EXEC} "$matrix_path" "$threads" "$schedule" "$chunk" 2>&1 | \
                grep -E "^[0-9]+\.[0-9]+$" | head -n1)
        perf_mode="no"
        metric_type="time_sec"
    fi

    echo "${matrix},${mode},${opt},${schedule},${chunk},${threads},${perf_mode},${metric_type},${result}" >> "$OUTPUT"
    
    if [ "$use_perf" = "yes" ]; then
        echo "  → [perf] L1-dcache-load-misses: ${result}"
    else
        echo "  → [time] ${result} sec"
    fi
}

# ==========================
# PARTE 1: SEQUENZIALE
# ==========================
echo ""
echo "════════════════════════════════════════════════════════════════"
echo "  PARTE SEQUENZIALE"
echo "════════════════════════════════════════════════════════════════"

for matrix in "${MATRICES[@]}"; do
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Matrix: ${matrix}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    for opt in "${OPT_FLAGS[@]}"; do
        opt_display=${opt:-"none"}
        echo ""
        echo "Optimization: ${opt_display}"
        
        # Compila SENZA -fopenmp per sequenziale
        COMPILE_CMD="${CC} ${opt} -Wall -g -std=c99 -IHeader ${LIBS} -o ${EXEC} ${SRC}"
        echo "  Compiling: ${COMPILE_CMD}"
        ${COMPILE_CMD} 2>/dev/null
        
        if [ $? -ne 0 ]; then
            echo "  ✗ Compilation failed!"
            continue
        fi
        
        echo "  Testing without perf..."
        run_test "$matrix" "sequential" "${opt_display}" "none" "none" 1 "no"
        
        echo "  Testing with perf..."
        run_test "$matrix" "sequential" "${opt_display}" "none" "none" 1 "yes"
    done
done

# ==========================
# PARTE 2: PARALLELA
# ==========================
echo ""
echo ""
echo "════════════════════════════════════════════════════════════════"
echo "  PARTE PARALLELA"
echo "════════════════════════════════════════════════════════════════"

# Compila con -O3 e tutti i flag come da Makefile
COMPILE_CMD="${CC} -O3 ${CFLAGS_BASE} ${LIBS} -o ${EXEC} ${SRC}"
echo ""
echo "Compiling parallel version: ${COMPILE_CMD}"
${COMPILE_CMD} 2>/dev/null

if [ $? -ne 0 ]; then
    echo "✗ Parallel compilation failed! Exiting."
    exit 1
fi

total_tests=$((${#MATRICES[@]} * ${#SCHEDULES[@]} * ${#CHUNKSIZES[@]} * ${#THREADS[@]} * 2))
current_test=0

for matrix in "${MATRICES[@]}"; do
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Matrix: ${matrix}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    for schedule in "${SCHEDULES[@]}"; do
        echo ""
        echo "Schedule: ${schedule}"
        
        for chunk in "${CHUNKSIZES[@]}"; do
            echo "  Chunk: ${chunk}"
            
            for threads in "${THREADS[@]}"; do
                current_test=$((current_test + 2))
                printf "    [%d/%d] Threads=%d " $current_test $total_tests $threads
                
                run_test "$matrix" "parallel" "-O3" "$schedule" "$chunk" "$threads" "no"
                
                printf "    [%d/%d] Threads=%d " $current_test $total_tests $threads
                
                run_test "$matrix" "parallel" "-O3" "$schedule" "$chunk" "$threads" "yes"
            done
        done
    done
done

# ==========================
# RIEPILOGO FINALE
# ==========================
echo ""
echo ""
echo "════════════════════════════════════════════════════════════════"
echo "  ✅ BENCHMARK COMPLETATO"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "Risultati salvati in: $OUTPUT"
echo ""

num_lines=$(wc -l < "$OUTPUT")
num_tests=$((num_lines - 1))
echo "Totale test eseguiti: ${num_tests}"

echo ""
echo "Prime 10 righe dei risultati:"
echo "────────────────────────────────────────────────────────────────"
head -11 "$OUTPUT" | column -t -s,
echo ""
