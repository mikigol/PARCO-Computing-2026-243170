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

.PHONY: all clean perf_csr_seq perf_csr_par perf_csb_seq perf_csb_par

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


