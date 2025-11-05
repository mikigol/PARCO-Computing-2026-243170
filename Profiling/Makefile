CC = gcc
CFLAGS = -Wall -O3 -fopenmp -std=c99 -I../Header
LIBS = -lm

# Parametri personalizzabili
MATRIX ?= ../Matrix/bcsstk13.mtx
THREADS ?= 4
SCHEDULE ?= static

# Sorgenti (da cartelle parallele)
CSR_SEQ_SRC = csr_seq_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c
CSR_PAR_SRC = csr_par_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c
CSB_SEQ_SRC = csb_seq_bench.c ../Src/matrix_io.c ../Src/csb.c ../Src/mmio.c
CSB_PAR_SRC = csb_par_bench.c ../Src/matrix_io.c ../Src/csb.c ../Src/mmio.c

.PHONY: all clean perf_csr_seq perf_csr_par perf_csb_seq perf_csb_par help

all: csr_seq_bench csr_par_bench csb_seq_bench csb_par_bench

csr_seq_bench: $(CSR_SEQ_SRC)
	$(CC) $(CFLAGS) -o csr_seq_bench $(CSR_SEQ_SRC) $(LIBS)

csr_par_bench: $(CSR_PAR_SRC)
	$(CC) $(CFLAGS) -o csr_par_bench $(CSR_PAR_SRC) $(LIBS)

csb_seq_bench: $(CSB_SEQ_SRC)
	$(CC) $(CFLAGS) -o csb_seq_bench $(CSB_SEQ_SRC) $(LIBS)

csb_par_bench: $(CSB_PAR_SRC)
	$(CC) $(CFLAGS) -o csb_par_bench $(CSB_PAR_SRC) $(LIBS)

perf_csr_seq:
	perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
	./csr_seq_bench $(MATRIX)

perf_csr_par:
	OMP_NUM_THREADS=$(THREADS) OMP_SCHEDULE=$(SCHEDULE) \
	perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
	./csr_par_bench $(MATRIX) $(THREADS) $(SCHEDULE)

perf_csb_seq:
	perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
	./csb_seq_bench $(MATRIX)

perf_csb_par:
	OMP_NUM_THREADS=$(THREADS) OMP_SCHEDULE=$(SCHEDULE) \
	perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
	./csb_par_bench $(MATRIX) $(THREADS) $(SCHEDULE)

clean:
	rm -f csr_seq_bench csr_par_bench csb_seq_bench csb_par_bench

help:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║        SpMV Benchmark: CSR vs CSB (Sequential & Parallel)      ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "TARGETS:"
	@echo "  make all              - Compila tutti gli eseguibili"
	@echo "  make clean            - Rimuove gli eseguibili"
	@echo ""
	@echo "PROFILING CON PERF:"
	@echo "  make perf_csr_seq     - CSR Sequenziale (baseline)"
	@echo "  make perf_csr_par     - CSR Parallelo (OpenMP)"
	@echo "  make perf_csb_seq     - CSB Sequenziale (baseline)"
	@echo "  make perf_csb_par     - CSB Parallelo (OpenMP)"
	@echo ""
	@echo "VARIABILI CONFIGURABILI (default in parentesi):"
	@echo "  MATRIX   - Percorso della matrice sparse (../Matrix/bcsstk13.mtx)"
	@echo "  THREADS  - Numero di thread OpenMP (4)"
	@echo "  SCHEDULE - Scheduling OpenMP: static, dynamic, guided (static)"
	@echo ""
	@echo "ESEMPI DI UTILIZZO:"
	@echo "  Compilazione:"
	@echo "    make all"
	@echo ""
	@echo "  Profiling CSR parallelo (16 thread, static scheduling):"
	@echo "    make perf_csr_par THREADS=16 SCHEDULE=static"
	@echo ""
	@echo "  Profiling CSB parallelo con diversa matrice:"
	@echo "    make perf_csb_par THREADS=16 SCHEDULE=guided MATRIX=../Matrix/bcsstk25.mtx"
	@echo ""
	@echo "  Profiling sequenziale (baseline per confronto):"
	@echo "    make perf_csr_seq MATRIX=../Matrix/bcsstk25.mtx"
	@echo ""
	@echo "SCHEDULE OPTIONS:"
	@echo "  static   - Pre-divide il lavoro equamente tra i thread (default, prevedibile)"
	@echo "  dynamic  - Distribuisce il lavoro dinamicamente (overhead, per load variabile)"
	@echo "  guided   - Ibrido: dinamico all'inizio, statico dopo (compromesso)"
	@echo ""
	@echo "METRICHE RACCOLTE:"
	@echo "  L1-dcache-loads        - Numero totale di accessi alla cache L1"
	@echo "  L1-dcache-load-misses  - Miss rate L1 (basso è meglio: <10%)"
	@echo "  LLC-loads              - Accessi alla Last Level Cache (L3)"
	@echo "  LLC-load-misses        - Miss rate L3 (indica accessi a RAM)"
	@echo ""
	@echo "INTERPRETAZIONE RISULTATI:"
	@echo "  L1 miss rate basso (<10%)  → Buona località spaziale/temporale"
	@echo "  L3 miss rate alto (>40%)   → Memory-bound, bottleneck RAM"
	@echo "  Speedup < thread count     → Contention su memoria, non CPU-bound"
	@echo ""
