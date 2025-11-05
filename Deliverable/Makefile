CC = gcc
CFLAGS = -Wall -g -O3 -fopenmp -std=c99
LIBS = -lm

# Variabili configurabili
MATRIX ?= bcsstk13.mtx
THREADS ?= 4
SCHEDULE ?= static


SRCS = main.c matrix_io.c csb.c mmio.c csr.c
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
	OMP_SCHEDULE=$(SCHEDULE) ./$(TARGET) "$(MATRIX)" $(THREADS)

help:
	@echo "=== Sparse Matrix Multiplication ==="
	@echo "Targets:"
	@echo "  make all     - Compila il programma"
	@echo "  make run     - Esegue, default: matrice 'bcsstk13.mtx' e 4 thread"
	@echo "  make clean   - Rimuove file oggetto e eseguibile"
	@echo "Variabili (esempio di uso: make run MATRIX=mymat.mtx THREADS=8):"
	@echo "  MATRIX  - nome file matrice (default: bcsstk13.mtx)"
	@echo "  THREADS - numero thread da passare al programma (default: 4)"





