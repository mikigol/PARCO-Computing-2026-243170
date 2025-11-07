CC = gcc
CFLAGS = -Wall -g -O3 -fopenmp -std=c99 -IHeader
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

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
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
	@echo ""
	@echo "Esempi:"
	@echo "  make run MATRIX=Matrix/g7jac200.mtx THREADS=8"
	@echo "  make run MATRIX=Matrix/bcsstk25.mtx THREADS=16"
	@echo ""
	@echo "NOTA: Il programma testa automaticamente:"
	@echo "      - 3 schedule types: static, dynamic, guided"
	@echo "      - 3 chunk sizes: 10, 100, 1000"
	@echo "      - 10 runs x 10000 iterations per test"
	@echo "      - Report con 90% percentile per ogni combinazione"

