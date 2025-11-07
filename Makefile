CC = gcc
# OPTIMIZATION: -O0 (none), -O1 (basic), -O2 (moderate), -O3 (aggressive)
OPT ?= -O3
CFLAGS = -Wall -g $(OPT) -fopenmp -std=c99 -IHeader
LIBS = -lm

# Variabili configurabili
MATRIX ?= Matrix/bcsstk13.mtx
THREADS ?= 4

# File sorgente nella cartella Src/
SRCS = Src/main.c Src/matrix_io.c Src/mmio.c Src/csr.c
OBJS = $(SRCS:.c=.o)
TARGET = esegui

.PHONY: all clean run help

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

run: $(TARGET)
	@echo "Running with $(OPT) optimization..."
	./$(TARGET) "$(MATRIX)" $(THREADS)

help:
	@echo "=== CSR SpMV Benchmark (Schedule Comparison) ==="
	@echo ""
	@echo "Targets:"
	@echo "  make all     - Compila il programma"
	@echo "  make run     - Esegue il benchmark completo"
	@echo "  make clean   - Rimuove file oggetto e eseguibile"
	@echo ""
	@echo "Variabili:"
	@echo "  MATRIX   - percorso file matrice (default: Matrix/bcsstk13.mtx)"
	@echo "  THREADS  - numero thread (default: 4)"
	@echo "  OPT      - livello ottimizzazione: -O0, -O1, -O2, -O3 (default: -O3)"
	@echo ""
	@echo "Esempi:"
	@echo "  make run MATRIX=Matrix/g7jac200.mtx THREADS=8"
	@echo "  make run MATRIX=Matrix/g7jac200.mtx THREADS=8 OPT=-O2"
	@echo "  make run OPT=-O0    # Nessuna ottimizzazione"
	@echo "  make run OPT=-O1    # Ottimizzazione base"
	@echo "  make run OPT=-O3    # Ottimizzazione aggressiva (default)"
	@echo ""
	@echo "Livelli di ottimizzazione GCC:"
	@echo "  -O0  : Nessuna ottimizzazione (debug)"
	@echo "  -O1  : Ottimizzazioni base (veloce compilazione)"
	@echo "  -O2  : Ottimizzazioni moderate (raccomandato produzione)"
	@echo "  -O3  : Ottimizzazioni aggressive (massima velocità)"
	@echo ""
	@echo "NOTA: Il programma testa automaticamente:"
	@echo "      - 3 schedule types: static, dynamic, guided"
	@echo "      - 3 chunk sizes: 10, 100, 1000"
	@echo "      - 10 runs x 10000 iterations per test"
	@echo "      - Report con 90% percentile per ogni combinazione"
