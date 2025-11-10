#!/bin/bash

# ==========================
# CONFIGURAZIONE
# ==========================
SRC="Src/main.c Src/matrix_io.c Src/csr.c Src/mmio.c"
EXEC="./matvec"
OUTPUT="results.csv"
MATRIX_DIR="Matrix"

# Flag di compilazione
CC="gcc"
CFLAGS_BASE="-Wall -g -fopenmp -std=c99 -IHeader"
LIBS="-lm"

# Matrici da testare (dentro Matrix/)
MATRICES=("bcsstk16.mtx" "bcsstk18.mtx" "bcsstk mtx k25..mtx")
OPT_FLAGS=("-O0" "-O1" "-O2" "-O3" "-Ofast")
SCHEDULES=("static" "dynamic" "guided")
CHUNKSIZES=(1 10 100 1000)
THREADS=(1 2 4 8 16 32 64)
NRUNS=10  # Numero di run per ogni combinazione (timing E perf)

# ==========================
# INIZIALIZZAZIONE CSV
# ==========================
echo "matrix,mode,opt_level,schedule,chunk_size,num_threads,run_number,perf_mode,metric_type,result_value,l1_loads,l1_misses,miss_rate_percent,llc_misses" > "$OUTPUT"

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
    local result_value=""
    local l1_loads=""
    local l1_misses=""
    local miss_rate=""
    local llc_misses=""

    if [ "$use_perf" = "yes" ]; then
        # Con perf: 10 run anche per perf
        perf_mode="yes"
        metric_type="L1_misses"
        
        echo -n "  → [perf runs] "
        
        for run in $(seq 1 $NRUNS); do
            perf_output=$(perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
                     ${EXEC} "$matrix_path" "$threads" "$schedule" "$chunk" 2>&1)
            
            # Estrai i valori singoli
            l1_loads=$(echo "$perf_output" | grep "L1-dcache-loads" | grep -v "load-misses" | awk '{print $1}' | sed 's/,//g')
            l1_misses=$(echo "$perf_output" | grep "L1-dcache-load-misses" | awk '{print $1}' | sed 's/,//g')
            llc_misses=$(echo "$perf_output" | grep "LLC-load-misses" | awk '{print $1}' | sed 's/,//g')
            
            # Calcola miss rate (%)
            if [ ! -z "$l1_loads" ] && [ "$l1_loads" -gt 0 ]; then
                miss_rate=$(echo "$l1_misses $l1_loads" | awk '{printf "%.4f", ($1 * 100) / $2}')
            else
                miss_rate="0.0000"
            fi
            
            result_value="${l1_misses}"
            
            # Salva questa run nel CSV
            echo "${matrix},${mode},${opt},${schedule},${chunk},${threads},${run},${perf_mode},${metric_type},${result_value},${l1_loads},${l1_misses},${miss_rate},${llc_misses}" >> "$OUTPUT"
            
            # Stampa inline
            printf "R%d:L1miss=%s(%.2f%%) " $run $l1_misses $miss_rate
        done
        
        echo ""  # A capo dopo tutte le run perf
        
    else
        # Senza perf: 10 run per timing
        l1_loads="N/A"
        l1_misses="N/A"
        miss_rate="N/A"
        llc_misses="N/A"
        perf_mode="no"
        metric_type="time_sec"
        
        echo -n "  → [time runs] "
        
        for run in $(seq 1 $NRUNS); do
            time_val=$(${EXEC} "$matrix_path" "$threads" "$schedule" "$chunk" 2>&1 | \
                      grep -E "^[0-9]+\.[0-9]+$" | head -n1)
            
            if [ ! -z "$time_val" ]; then
                result_value="$time_val"
                
                # Salva questa run nel CSV
                echo "${matrix},${mode},${opt},${schedule},${chunk},${threads},${run},${perf_mode},${metric_type},${result_value},${l1_loads},${l1_misses},${miss_rate},${llc_misses}" >> "$OUTPUT"
                
                # Stampa inline
                printf "R%d:%.6f " $run $time_val
            else
                echo "${matrix},${mode},${opt},${schedule},${chunk},${threads},${run},${perf_mode},${metric_type},ERROR,${l1_loads},${l1_misses},${miss_rate},${llc_misses}" >> "$OUTPUT"
                printf "R%d:ERROR " $run
            fi
        done
        
        echo ""  # A capo dopo tutte le run
    fi
}

# ==========================
# PARTE 1: SEQUENZIALE
# ==========================
echo ""
echo "════════════════════════════════════════════════════════════════"
echo "  PARTE SEQUENZIALE (con ${NRUNS} run per timing E perf)"
echo "════════════════════════════════════════════════════════════════"

for matrix in "${MATRICES[@]}"; do
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Matrix: ${matrix}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    for opt in "${OPT_FLAGS[@]}"; do
        opt_display=${opt}
        echo ""
        echo "Optimization: ${opt_display}"
        
        # Compila
        COMPILE_CMD="${CC} ${opt} -Wall -g -std=c99 -IHeader -fopenmp ${LIBS} -o ${EXEC} ${SRC}"
        echo "  Compiling: ${COMPILE_CMD}"
        ${COMPILE_CMD} 2>/dev/null
        
        if [ $? -ne 0 ]; then
            echo "  ✗ Compilation failed!"
            continue
        fi
        
        echo "  Testing without perf (${NRUNS} runs)..."
        run_test "$matrix" "sequential" "${opt_display}" "none" "none" 1 "no"
        
        echo "  Testing with perf (${NRUNS} runs)..."
        run_test "$matrix" "sequential" "${opt_display}" "none" "none" 1 "yes"
    done
done

# ==========================
# PARTE 2: PARALLELA
# ==========================
echo ""
echo ""
echo "════════════════════════════════════════════════════════════════"
echo "  PARTE PARALLELA (con ${NRUNS} run per timing E perf)"
echo "════════════════════════════════════════════════════════════════"

# Compila con -O3
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
                printf "    [%d/%d] Threads=%d\n" $current_test $total_tests $threads
                
                run_test "$matrix" "parallel" "-O3" "$schedule" "$chunk" "$threads" "no"
                
                printf "    [%d/%d] Threads=%d\n" $current_test $total_tests $threads
                
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
echo "Totale righe nel CSV: ${num_tests}"

echo ""
echo "Prime 25 righe dei risultati:"
echo "────────────────────────────────────────────────────────────────"
head -26 "$OUTPUT" | column -t -s,
echo ""

echo "Note:"
echo "  - Ogni combinazione timing: ${NRUNS} righe (una per run)"
echo "  - Ogni combinazione perf: ${NRUNS} righe (una per run)"
echo "  - Totale righe per combinazione: $((NRUNS * 2)) righe"
echo ""
