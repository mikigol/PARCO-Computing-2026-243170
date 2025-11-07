CC = gcc
CFLAGS = -Wall -O3 -fopenmp -std=c99 -I../Header
LIBS = -lm

# Parametri personalizzabili
MATRIX ?= ../Matrix/bcsstk13.mtx
THREADS ?= 4
SCHEDULE ?= 0
CHUNK ?= 1000

# Sorgenti
CSR_SEQ_SRC = csr_seq_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c
CSR_PAR_SRC = csr_par_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c

.PHONY: all clean perf_csr_seq perf_csr_par help

all: csr_seq_bench csr_par_bench

csr_seq_bench: $(CSR_SEQ_SRC)
	$(CC) $(CFLAGS) -o csr_seq_bench $(CSR_SEQ_SRC) $(LIBS)

csr_par_bench: $(CSR_PAR_SRC)
	$(CC) $(CFLAGS) -o csr_par_bench $(CSR_PAR_SRC) $(LIBS)

perf_csr_seq:
	perf stat -e cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses \
	./csr_seq_bench "$(MATRIX)"

perf_csr_par:
	perf stat -e cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses \
	./csr_par_bench "$(MATRIX)" $(THREADS) $(SCHEDULE) $(CHUNK)

clean:
	rm -f csr_seq_bench csr_par_bench

help:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║              CSR SpMV Profiling with PERF                     ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "TARGETS:"
	@echo "  make all              - Compila entrambi gli eseguibili"
	@echo "  make perf_csr_seq     - CSR Sequenziale (baseline)"
	@echo "  make perf_csr_par     - CSR Parallelo (OpenMP)"
	@echo "  make clean            - Rimuove gli eseguibili"
	@echo ""
	@echo "VARIABILI CONFIGURABILI (default in parentesi):"
	@echo "  MATRIX   - Percorso della matrice sparse (../Matrix/bcsstk13.mtx)"
	@echo "  THREADS  - Numero di thread OpenMP (4)"
	@echo "  SCHEDULE - OpenMP schedule: 0=static, 1=dynamic, 2=guided (0=static)"
	@echo "  CHUNK    - Chunk size (default: 1000)"
	@echo ""
	@echo "ESEMPI DI UTILIZZO:"
	@echo "  Compilazione:"
	@echo "    make all"
	@echo ""
	@echo "  Profiling sequenziale (baseline):"
	@echo "    make perf_csr_seq MATRIX=../Matrix/bcsstk13.mtx"
	@echo ""
	@echo "  Profiling parallelo (8 thread, static, chunk 1000):"
	@echo "    make perf_csr_par MATRIX=../Matrix/bcsstk13.mtx THREADS=8 SCHEDULE=0 CHUNK=1000"
	@echo ""
	@echo "  Profiling parallelo (8 thread, dynamic, chunk 100):"
	@echo "    make perf_csr_par MATRIX=../Matrix/g7jac200.mtx THREADS=8 SCHEDULE=1 CHUNK=100"
	@echo ""
	@echo "  Profiling parallelo (16 thread, guided, chunk 10):"
	@echo "    make perf_csr_par MATRIX=../Matrix/bcsstk25.mtx THREADS=16 SCHEDULE=2 CHUNK=10"
	@echo ""
	@echo "SCHEDULE OPTIONS:"
	@echo "  0 (static)   - Pre-divide il lavoro equamente tra i thread"
	@echo "  1 (dynamic)  - Distribuisce il lavoro dinamicamente (overhead)"
	@echo "  2 (guided)   - Ibrido: dinamico iniziale, statico finale"
	@echo ""
	@echo "METRICHE RACCOLTE:"
	@echo "  cache-references       - Accessi totali alla cache"
	@echo "  cache-misses           - Cache miss rate (%)"
	@echo "  L1-dcache-load-misses  - L1 miss (basso è meglio: <10%)"
	@echo "  LLC-load-misses        - L3 miss (indica accessi a RAM)"
	@echo ""
	@echo "INTERPRETAZIONE RISULTATI:"
	@echo "  L1 miss rate basso (<10%)   → Buona località spaziale/temporale"
	@echo "  L3 miss rate alto (>40%)    → Memory-bound, bottleneck RAM"
	@echo "  Speedup < thread count      → Contention memoria, non CPU-bound"
	@echo ""

