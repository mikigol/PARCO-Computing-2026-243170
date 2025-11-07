CC = gcc
OPT ?= -O3
CFLAGS = -Wall -g $(OPT) -fopenmp -std=c99 -IHeader
LIBS = -lm

MATRIX ?= Matrix/bcsstk13.mtx
THREADS ?= 4

SRCS = Src/main.c Src/matrix_io.c Src/mmio.c Src/csr.c
OBJS = $(SRCS:.c=.o)
TARGET = esegui

.PHONY: all clean run build help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo ""
	@echo "Compiled with optimization level: $(OPT)"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned up object files and executable"

build: clean all
	@echo "Clean rebuild completed"

run: $(TARGET)
	@echo "Running with $(OPT) optimization..."
	./$(TARGET) "$(MATRIX)" $(THREADS)

help:
	@echo "=== CSR SpMV Benchmark (Optimization Levels) ==="
	@echo ""
	@echo "Targets:"
	@echo "  make build   - Ricompila completamente da zero"
	@echo "  make run     - Esegue il benchmark"
	@echo "  make clean   - Rimuove file compilati"
	@echo ""
	@echo "Variabili:"
	@echo "  MATRIX - percorso file matrice (default: Matrix/bcsstk13.mtx)"
	@echo "  THREADS - numero thread (default: 4)"
	@echo "  OPT - ottimizzazione: -O0, -O1, -O2, -O3 (default: -O3)"
	@echo ""
	@echo "CORRETTO - Per cambiare ottimizzazione:"
	@echo "  make build OPT=-O3"
	@echo "  make run MATRIX=Matrix/g7jac200.mtx THREADS=8"
	@echo ""
	@echo "  make build OPT=-O0"
	@echo "  make run MATRIX=Matrix/g7jac200.mtx THREADS=8"
	@echo ""
	@echo "  make build OPT=-O2"
	@echo "  make run MATRIX=Matrix/bcsstk25.mtx THREADS=16"
	@echo ""
	@echo "Livelli di ottimizzazione GCC:"
	@echo "  -O0 : Nessuna ottimizzazione (debug)"
	@echo "  -O1 : Ottimizzazioni base"
	@echo "  -O2 : Ottimizzazioni moderate (raccomandato)"
	@echo "  -O3 : Ottimizzazioni aggressive (massima velocità)"
