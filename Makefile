CC = gcc
CFLAGS = -Wall -g -O3 -fopenmp -std=c99 -IHeader
LIBS = -lm

# Variabili configurabili
MATRIX ?= Matrix/bcsstk13.mtx
THREADS ?= 4
SCHEDULE ?= static

# File sorgente nella cartella Src/
SRCS = Src/main.c Src/matrix_io.c  Src/mmio.c Src/csr.c
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
	OMP_NUM_THREADS=$(THREADS) OMP_SCHEDULE=$(SCHEDULE) ./$(TARGET) "$(MATRIX)" $(THREADS)

help:
	@echo "=== Sparse Matrix Multiplication ==="
	@echo "Targets:"
	@echo "  make all     - Compila il programma"
	@echo "  make run     - Esegue con default"
	@echo "  make clean   - Rimuove file oggetto e eseguibile"
	@echo ""
	@echo "Variabili (esempio: make run MATRIX=Matrix/bcsstk25.mtx THREADS=8 SCHEDULE=static):"
	@echo "  MATRIX   - percorso file matrice (default: Matrix/bcsstk13.mtx)"
	@echo "  THREADS  - numero thread (default: 4)"
	@echo "  SCHEDULE - schedule OpenMP: static, dynamic, guided (default: static)"
