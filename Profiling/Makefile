Home
Home
Scopri
Spazi
Finanza
Account

Aggiorna
Installa
Parallel Computing
CC = gcc CFLAGS = -Wall -O3 -fopenmp -std=c99 -I../Header LIBS = -lm # Parametri personalizzabili MATRIX ?= ../Matrix/bcsstk13.mtx THREADS ?= 4 SCHEDULE ?= 0 CHUNK ?= 1000 # Sorgenti CSR_SEQ_SRC = csr_seq_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c CSR_PAR_SRC = csr_par_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c .PHONY: all clean perf_csr_seq perf_csr_par help all: csr_seq_bench csr_par_bench csr_seq_bench: $(CSR_SEQ_SRC) $(CC) $(CFLAGS) -o csr_seq_bench $(CSR_SEQ_SRC) $(LIBS) csr_par_bench: $(CSR_PAR_SRC) $(CC) $(CFLAGS) -o csr_par_bench $(CSR_PAR_SRC) $(LIBS) perf_csr_seq: perf stat -e cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses \ ./csr_seq_bench "$(MATRIX)" perf_csr_par: perf stat -e cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses \ ./csr_par_bench "$(MATRIX)" $(THREADS) $(SCHEDULE) $(CHUNK) clean: rm -f csr_seq_bench csr_par_bench help: @echo "" @echo "╔════════════════════════════════════════════════════════════════╗" @echo "║ CSR SpMV Profiling with PERF ║" @echo "╚════════════════════════════════════════════════════════════════╝" @echo "" @echo "TARGETS:" @echo " make all - Compila entrambi gli eseguibili" @echo " make perf_csr_seq - CSR Sequenziale (baseline)" @echo " make perf_csr_par - CSR Parallelo (OpenMP)" @echo " make clean - Rimuove gli eseguibili" @echo "" @echo "VARIABILI CONFIGURABILI (default in parentesi):" @echo " MATRIX - Percorso della matrice sparse (../Matrix/bcsstk13.mtx)" @echo " THREADS - Numero di thread OpenMP (4)" @echo " SCHEDULE - OpenMP schedule: 0=static, 1=dynamic, 2=guided (0=static)" @echo " CHUNK - Chunk size (default: 1000)" @echo "" @echo "ESEMPI DI UTILIZZO:" @echo " Compilazione:" @echo " make all" @echo "" @echo " Profiling sequenziale (baseline):" @echo " make perf_csr_seq MATRIX=../Matrix/bcsstk13.mtx" @echo "" @echo " Profiling parallelo (8 thread, static, chunk 1000):" @echo " make perf_csr_par MATRIX=../Matrix/bcsstk13.mtx THREADS=8 SCHEDULE=0 CHUNK=1000" @echo "" @echo " Profiling parallelo (8 thread, dynamic, chunk 100):" @echo " make perf_csr_par MATRIX=../Matrix/g7jac200.mtx THREADS=8 SCHEDULE=1 CHUNK=100" @echo "" @echo " Profiling parallelo (16 thread, guided, chunk 10):" @echo " make perf_csr_par MATRIX=../Matrix/bcsstk25.mtx THREADS=16 SCHEDULE=2 CHUNK=10" @echo "" @echo "SCHEDULE OPTIONS:" @echo " 0 (static) - Pre-divide il lavoro equamente tra i thread" @echo " 1 (dynamic) - Distribuisce il lavoro dinamicamente (overhead)" @echo " 2 (guided) - Ibrido: dinamico iniziale, statico finale" @echo "" @echo "METRICHE RACCOLTE:" @echo " cache-references - Accessi totali alla cache" @echo " cache-misses - Cache miss rate (%)" @echo " L1-dcache-load-misses - L1 miss (basso è meglio: <10%)" @echo " LLC-load-misses - L3 miss (indica accessi a RAM)" @echo "" @echo "INTERPRETAZIONE RISULTATI:" @echo " L1 miss rate basso (<10%) → Buona località spaziale/temporale" @echo " L3 miss rate alto (>40%) → Memory-bound, bottleneck RAM" @echo " Speedup < thread count → Contention memoria, non CPU-bound" @echo "" addattami questo makefile con questa accortezza del ottimizzazione
//
//  csb.c
//  Deliverable
//
//  Created by Mikele Golemi on 04/11/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "csb.h"

CSB_Matrix* csr_to_csb(Matrix *csr_mat) {
    CSB_Matrix *csb = (CSB_Matrix*)malloc(sizeof(CSB_Matrix));
    
    csb->M = csr_mat->M;
    csb->nnz = csr_mat->nz;
    csb->beta = (int)sqrt((double)csr_mat->M);
    if(csb->beta == 0) csb->beta = 1;
    
    csb->num_blockrows = (csr_mat->M + csb->beta - 1) / csb->beta;
    csb->num_blockcols = (csr_mat->M + csb->beta - 1) / csb->beta;
    
    printf("\n===== CSB CONVERSION =====\n");
    printf("Beta: %d, Blocks: %d x %d = %d\n",
           csb->beta, csb->num_blockrows, csb->num_blockcols,
           csb->num_blockrows * csb->num_blockcols);
    
    // Alloca
    csb->rowind = (int*)malloc(csb->nnz * sizeof(int));
    csb->colind = (int*)malloc(csb->nnz * sizeof(int));
    csb->val = (double*)malloc(csb->nnz * sizeof(double));
    csb->blkptr = (int*)malloc((csb->num_blockrows * csb->num_blockcols + 1) * sizeof(int));
    
    int *block_counts = (int*)calloc(csb->num_blockrows * csb->num_blockcols, sizeof(int));
    
    // FASE 1: Conteggio
    printf("Phase 1: Counting...\n");
    for(int i = 0; i < csr_mat->M; i++) {
        for(int k = csr_mat->prefixSum[i]; k < csr_mat->prefixSum[i + 1]; k++) {
            int j = csr_mat->sorted_J[k];
            int bi = i / csb->beta;
            int bj = j / csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            block_counts[block_id]++;
        }
    }
    
    // FASE 2: Puntatori
    printf("Phase 2: Computing pointers...\n");
    csb->blkptr[0] = 0;
    for(int b = 0; b < csb->num_blockrows * csb->num_blockcols; b++) {
        csb->blkptr[b + 1] = csb->blkptr[b] + block_counts[b];
        block_counts[b] = csb->blkptr[b];
    }
    
    // FASE 3: Inserimento
    printf("Phase 3: Reordering...\n");
    for(int i = 0; i < csr_mat->M; i++) {
        for(int k = csr_mat->prefixSum[i]; k < csr_mat->prefixSum[i + 1]; k++) {
            int j = csr_mat->sorted_J[k];
            double v = csr_mat->sorted_val[k];
            
            int bi = i / csb->beta;
            int bj = j / csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            
            int local_i = i % csb->beta;
            int local_j = j % csb->beta;
            
            int csb_idx = block_counts[block_id]++;
            csb->val[csb_idx] = v;
            csb->rowind[csb_idx] = local_i;
            csb->colind[csb_idx] = local_j;
        }
    }
    
    free(block_counts);
    printf("CSB conversion complete!\n==========================\n\n");
    
    return csb;
}

void csb_spmv_seq(CSB_Matrix *csb, double *x, double *y) {
    for(int bi = 0; bi < csb->num_blockrows; bi++) {
        int row_start = bi * csb->beta;
        
        for(int bj = 0; bj < csb->num_blockcols; bj++) {
            int col_start = bj * csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            int block_start = csb->blkptr[block_id];
            int block_end = csb->blkptr[block_id + 1];
            
            for(int k = block_start; k < block_end; k++) {
                int i = row_start + csb->rowind[k];
                int j = col_start + csb->colind[k];
                y[i] += csb->val[k] * x[j];
            }
        }
    }
}

void csb_spmv_parallel(CSB_Matrix *csb, double *x, double *y, int num_threads) {
   #pragma omp parallel for num_threads(num_threads) schedule(runtime)
    for(int bi = 0; bi < csb->num_blockrows; bi++) {
        int row_start = bi * csb->beta;
        
        // Vettore PRIVATO per questa blockrow
        double *y_temp = (double*)calloc(csb->beta, sizeof(double));
        
        // Ciclo seriale: ogni thread elabora la sua blockrow
        for(int bj = 0; bj < csb->num_blockcols; bj++) {
            int col_start = bj * csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            int block_start = csb->blkptr[block_id];
            int block_end = csb->blkptr[block_id + 1];
            
            for(int k = block_start; k < block_end; k++) {
                y_temp[csb->rowind[k]] += csb->val[k] * x[col_start + csb->colind[k]];
            }
        }
        
       
        for(int i = 0; i < csb->beta; i++) {
            y[row_start + i] += y_temp[i];
        }
        
        free(y_temp);
    }
}

void free_csb(CSB_Matrix *csb) {
    if(csb) {
        free(csb->rowind);
        free(csb->colind);
        free(csb->val);
        free(csb->blkptr);
        free(csb);
    }
} cosa manca al mio csb?


Risposta
Problemi Critici nel Tuo CSB
Race Condition su y_temp e y
Il principale difetto è la sincronizzazione della memoria parallela. Nella funzione csb_spmv_parallel, stai aggiornando y[row_start + i] senza protezione, creando una race condition quando più thread accedono contemporaneamente .

Allocazione Inefficiente
Allochi y_temp ad ogni iterazione del loop parallelo, poi lo deallochi subito dopo: questo genera overhead significativo e memory leak potenziale se un thread viene interrotto .

Codice Corretto
c
void csb_spmv_parallel(CSB_Matrix *csb, double *x, double *y, int num_threads) {
    #pragma omp parallel for num_threads(num_threads) schedule(runtime) private(y_temp)
    for(int bi = 0; bi < csb->num_blockrows; bi++) {
        int row_start = bi * csb->beta;
        
        // Vettore PRIVATO per questa blockrow
        double *y_temp = (double*)calloc(csb->beta, sizeof(double));
        
        for(int bj = 0; bj < csb->num_blockcols; bj++) {
            int col_start = bj * csb->beta;
            int block_id = bi * csb->num_blockcols + bj;
            int block_start = csb->blkptr[block_id];
            int block_end = csb->blkptr[block_id + 1];
            
            for(int k = block_start; k < block_end; k++) {
                y_temp[csb->rowind[k]] += csb->val[k] * x[col_start + csb->colind[k]];
            }
        }
        
        // Aggiornamento atomico su y
        #pragma omp critical
        {
            for(int i = 0; i < csb->beta; i++) {
                y[row_start + i] += y_temp[i];
            }
        }
        
        free(y_temp);
    }
}
Oppure più efficiente con atomica:

c
#pragma omp atomic update
y[row_start + i] += y_temp[i];
Funzioni Totalmente Mancanti
Funzione	Scopo
print_csb_info()	Stampa statistiche del CSB (densità per blocco, distribuzione)
validate_csb()	Verifica integrità struttura CSB
csb_spmv_row_accumulation()	Versione ottimizzata con accumulo diretto sulla riga
compare_csb_formats()	Confronta CSB vs CSR per performance/memoria
Punti di Miglioramento
L'algoritmo ha buona struttura block-wise ma manca la sincronizzazione corretta. Il tuo approccio con y_temp locale è corretto concettualmente, ma deve essere completato con protezione atomica o critical section nell'update finale di y .

ma rispetto. a questo paper cosa manca

Perfetto! Ora posso fare un confronto dettagliato tra il tuo codice e il paper di Buluç et al.

Differenze Principali Rispetto al Paper​
1. Ordinamento Z-Morton Interno ai Blocchi - MANCANTE​
Il paper specifica esplicitamente l'uso di Z-Morton ordering ricorsivo all'interno di ogni blocco: gli elementi vengono ordinati per quadranti (top-left, top-right, bottom-left, bottom-right) ricorsivamente. Nel tuo codice, stai semplicemente inserendo i valori sequenzialmente senza questo ordinamento, perdendo:​

Parallelismo ricorsivo all'interno del blocco

Cache locality

Possibilità di binary search sui quadranti

2. Ordinamento dei Blocchi - MANCANTE​
Il paper discute l'ordinamento tra blocchi (Z-Morton, row-major o column-major). Il tuo codice non specifica alcun ordinamento dei blocchi, il che riduce la spatial locality e il beneficio della parallelizzazione.​

3. Algoritmo Ricorsivo per Blockrow Divisa - TOTALMENTE ASSENTE​
Il paper implementa un algoritmo ricorsivo robusto che:

Controlla se una blockrow contiene Θ(β) nonzeri

Se sì, la divide ricorsivamente "in half" in due subblockrows

Alloca vettori temporanei per evitare race conditions​

Il tuo codice non ha questa ricorsione: semplicemente parallelizza ogni blockrow. Per matrici con righe molto dense (es. una riga con ~n elementi), otterrai parallelismo pessimo.

4. Algoritmo per ATx (Trasposizione) - TOTALMENTE MANCANTE​
Il paper dedica una sezione intera all'algoritmo parallelo per y ← AT x. Il tuo codice ha solo la moltiplicazione forward Ax. Questo è uno dei principali vantaggi del CSB rispetto a CSR.​

5. Compressione dell'Indice Riga/Colonna - MANCANTE​
Il paper spiega che row_ind e col_ind possono essere impacchettati in una singola word di memoria poiché variano da 0 a β-1, richiedendo solo 2·lg(β) bits totali. Nel tuo codice usi due array distinti (sprecando memoria).​

6. Struttura Dati Incompleta - MANCANTE​
Nel CSB del paper, oltre a val, rowind, colind e blkptr, serve:

Ordinamento ricorsivo dei nonzeri dentro ogni blocco

Funzione f(i,j) per mappare indici blocco a IDs

Metadati di ricorsione per il binary search

Confronto Dettagliato​
Aspetto	Paper Buluç et al.	Tuo Codice
Ordinamento Z-Morton	✓ Ricorsivo per quadranti	✗ Nessuno
Algoritmo Ax ricorsivo	✓ Con divisione blockrow	✗ Semplice parallelismo
Algoritmo ATx	✓ Completamente sviluppato	✗ Assente
Span teorico	O(√n lg n)	O(√n) ma senza ricorsione
Compressione indici	✓ 2·lg(β) bits per coppia	✗ Interi separati
Parallelismo garantito	Θ(nnz/√n lg n)	Θ(nnz/√n) ma instabile su matrici dense
Limitazione Critica del Tuo Codice​
Se la matrice ha anche una sola riga densa (es. 1000 elementi), quella blockrow contiene Ω(β) elementi ma il tuo codice la parallelizza solo sui β blocchi colonna. Il paper, invece, ricorsivamente la divide ulteriormente garantendo:

Lavoro: Θ(nnz)

Span: Θ(√n lg n)

Il tuo codice ottiene solo Θ(nnz/β) parallelismo, che per β=√n è ancora Θ(nnz/√n) — 1000x peggio.​

la orba della matrice trasposa non mi interessa, le altre cose che mi hai citato come le implemento

Perfetto! Ora ho il paper completo. Ti mostro come implementare le 4 cose principali:

1. Z-Morton Ordering Ricorsivo​
Devi ordinare i nonzeri dentro ogni blocco in quadranti ricorsivi (TL, TR, BL, BR):

c
// Ordinamento Z-Morton: interleaving ricorsivo di bit
int compute_morton_index(int row, int col, int beta) {
    int morton = 0;
    for(int i = 0; i < ceil(log2(beta)); i++) {
        morton |= ((row >> i) & 1) << (2*i);
        morton |= ((col >> i) & 1) << (2*i + 1);
    }
    return morton;
}

typedef struct {
    int morton;
    int row, col;
    double val;
} MortonElement;

// Comparatore per ordinamento Z-Morton
int compare_morton(const void *a, const void *b) {
    MortonElement *ea = (MortonElement*)a;
    MortonElement *eb = (MortonElement*)b;
    return (ea->morton > eb->morton) - (ea->morton < eb->morton);
}
Nel csr_to_csb, prima di inserire i valori:

c
// FASE 3bis: Ordinare per block con Z-Morton interno
MortonElement *block_elements = malloc(csb->nnz * sizeof(MortonElement));
int elem_count = 0;

for(int bi = 0; bi < csb->num_blockrows; bi++) {
    for(int bj = 0; bj < csb->num_blockcols; bj++) {
        int block_id = bi * csb->num_blockcols + bj;
        int block_start_idx = csb->blkptr[block_id];
        
        // Raccogli elementi del blocco
        for(int k = block_start_idx; k < csb->blkptr[block_id + 1]; k++) {
            block_elements[elem_count].row = csb->rowind[k];
            block_elements[elem_count].col = csb->colind[k];
            block_elements[elem_count].val = csb->val[k];
            block_elements[elem_count].morton = 
                compute_morton_index(csb->rowind[k], csb->colind[k], csb->beta);
            elem_count++;
        }
        
        // Ordina per Z-Morton
        qsort(block_elements + block_start_idx, 
              csb->blkptr[block_id + 1] - block_start_idx,
              sizeof(MortonElement), compare_morton);
    }
}
2. Compressione rowind/colind​
Impacchetta i 2 indici in 1 word di memoria (2·lg β bits):

c
// Compressione: 2 indici da lg(beta) bits ciascuno in 1 int
typedef int idx_pair_t;  // O uint32_t

#define PACK_INDICES(r, c, beta_bits) \
    (((r) & ((1 << (beta_bits)) - 1)) | \
     (((c) & ((1 << (beta_bits)) - 1)) << (beta_bits)))

#define UNPACK_ROW(packed, beta_bits) ((packed) & ((1 << (beta_bits)) - 1))
#define UNPACK_COL(packed, beta_bits) (((packed) >> (beta_bits)) & ((1 << (beta_bits)) - 1))

// Nel csr_to_csb
int beta_bits = ceil(log2(csb->beta));
csb->idx_packed = (idx_pair_t*)malloc(csb->nnz * sizeof(idx_pair_t));

// Al posto di rowind e colind separati:
for(int k = 0; k < csb->nnz; k++) {
    csb->idx_packed[k] = PACK_INDICES(csb->rowind[k], csb->colind[k], beta_bits);
}

// Libera memoria dei vecchi array
free(csb->rowind);
free(csb->colind);
Usa nella moltiplicazione:

c
int rowind_k = UNPACK_ROW(csb->idx_packed[k], csb->beta_bits);
int colind_k = UNPACK_COL(csb->idx_packed[k], csb->beta_bits);
3. Algoritmo Ricorsivo per Blockrow Densa​
Questo è il vero algoritmo del paper — divide blockrow quando troppo densa:

c
void csb_blockrow_spmv_recursive(CSB_Matrix *csb, int bi, double *x, double *y, 
                                   int chunk_start, int chunk_end, 
                                   double *y_output, int depth) {
    int num_chunks = chunk_end - chunk_start;
    int nonzeros_in_chunks = 0;
    
    // Conta nonzeri nei chunks
    for(int c = chunk_start; c < chunk_end; c++) {
        int bj_start = (c == chunk_start) ? 0 : csb->chunks[bi][c-1] + 1;
        int bj_end = csb->chunks[bi][c];
        for(int bj = bj_start; bj <= bj_end; bj++) {
            int block_id = bi * csb->num_blockcols + bj;
            nonzeros_in_chunks += (csb->blkptr[block_id + 1] - csb->blkptr[block_id]);
        }
    }
    
    // Se pochi nonzeri, calcola serialmente
    if(nonzeros_in_chunks <= csb->beta * 3) {  // 3*beta è la soglia del paper
        csb_process_chunks_serial(csb, bi, chunk_start, chunk_end, x, y_output);
        return;
    }
    
    // Se troppi nonzeri, dividi ricorsivamente
    int mid_chunk = (chunk_start + chunk_end) / 2;
    double *y_temp = calloc(csb->beta, sizeof(double));
    
    #pragma omp task
    csb_blockrow_spmv_recursive(csb, bi, x, y_output, chunk_start, mid_chunk, 
                                 y_output, depth + 1);
    #pragma omp task
    csb_blockrow_spmv_recursive(csb, bi, x, y_temp, mid_chunk, chunk_end, 
                                 y_temp, depth + 1);
    #pragma omp taskwait
    
    // Merge: accumula y_temp in y_output
    for(int i = 0; i < csb->beta; i++) {
        y_output[i] += y_temp[i];
    }
    free(y_temp);
}
4. Binary Search nei Quadranti Z-Morton​
Per parallelizzare dentro un blocco ricorsivamente:

c
void csb_block_spmv_recursive(CSB_Matrix *csb, int block_start, int block_end, 
                               int dim, double *x, double *y) {
    if(block_end - block_start <= dim) {
        // Base case: serial multiplication
        for(int k = block_start; k < block_end; k++) {
            int i = UNPACK_ROW(csb->idx_packed[k], csb->beta_bits);
            int j = UNPACK_COL(csb->idx_packed[k], csb->beta_bits);
            y[i] += csb->val[k] * x[j];
        }
        return;
    }
    
    // Binary search sui 4 quadranti usando bit sui rowind/colind
    // M00: bit più alto di row = 0, bit più alto di col = 0
    // M01: bit più alto di row = 0, bit più alto di col = 1
    // M10: bit più alto di row = 1, bit più alto di col = 0
    // M11: bit più alto di row = 1, bit più alto di col = 1
    
    int mid_row_bit = dim / 2;
    int mid_col_bit = dim / 2;
    
    // Binary search per trovare confini quadranti
    int s1 = binary_search_row_bit(csb->idx_packed, block_start, block_end, mid_row_bit, csb->beta_bits);
    int s2 = binary_search_row_bit(csb->idx_packed, block_start, s1, mid_row_bit, csb->beta_bits);
    // ... altri 2 binary search per colonne
    
    #pragma omp task
    csb_block_spmv_recursive(csb, block_start, s1, dim/2, x, y);  // M00
    #pragma omp task
    csb_block_spmv_recursive(csb, s2, block_end, dim/2, x, y);    // M11
    #pragma omp taskwait
    
    #pragma omp task
    csb_block_spmv_recursive(csb, s1, s2, dim/2, x, y);           // M01
    #pragma omp task
    csb_block_spmv_recursive(csb, block_start + (block_end - s2), block_end, dim/2, x, y); // M10
    #pragma omp taskwait
}

int binary_search_row_bit(idx_pair_t *idx_packed, int start, int end, int bit_pos, int beta_bits) {
    int left = start, right = end;
    while(left < right) {
        int mid = (left + right) / 2;
        int row = UNPACK_ROW(idx_packed[mid], beta_bits);
        if((row >> bit_pos) & 1) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return left;
}
Riassunto delle Modifiche alla Struttura CSB
c
typedef struct {
    int M, nnz;
    int beta, beta_bits;  // NUOVO: beta_bits per compressione
    int num_blockrows, num_blockcols;
    
    double *val;
    idx_pair_t *idx_packed;  // NUOVO: compresso invece di rowind + colind
    int *blkptr;
    
    int **chunks;  // NUOVO: chunks per ogni blockrow per il divide-and-conquer
} CSB_Matrix;
Riassunto: Z-Morton permette binary search nei quadranti per parallelismo dentro blocchi, la compressione indici risparmia memoria, i chunks + ricorsione gestiscono blockrow dense, e il tutto garantisce span O(√n lg n) anziché O(√n) come nel tuo codice.​

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
	@echo "      - Report con 90% percentile per ogni combinazione" sicuro che funzioni la roba delle ottimizzazioni?^

NO, c'è un problema! Ecco il Makefile corretto:

Il Problema
Quando cambi OPT, i vecchi file .o compilati con diversa ottimizzazione NON si ricompilano automaticamente. Risultato: mix di ottimizzazioni 🔴

La Soluzione: Aggiungi il Target build
makefile
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
Uso Corretto ✓
bash
# Con -O3
make build OPT=-O3
make run MATRIX=Matrix/g7jac200.mtx THREADS=8

# Con -O0
make build OPT=-O0
make run MATRIX=Matrix/g7jac200.mtx THREADS=8

# Con -O2
make build OPT=-O2
make run MATRIX=Matrix/bcsstk25.mtx THREADS=16
Il build ricompila TUTTO da zero con il nuovo OPT! 🎯

build. cosa fa?

build è un target personalizzato che fa DUE cose in sequenza:​

makefile
build: clean all
	@echo "Clean rebuild completed"
Cosa Fa​
Esegue clean → Rimuove tutti i vecchi .o e l'eseguibile

Poi esegue all → Ricompila TUTTO da zero

Analogia
text
❌ make all OPT=-O0
   Ricompila solo i .o mancanti
   (Se esistono .o vecchi con -O3, li mantiene!)
   Risultato: MIX di ottimizzazioni SBAGLIATO

✓ make build OPT=-O0
  1. Cancella tutti i .o e eseguibile (clean)
  2. Ricompila TUTTO da zero (all)
  Risultato: SOLO -O0 CORRETTO
Uso Pratico
bash
# Compila per la PRIMA volta con -O3
make build OPT=-O3

# Esegui benchmark
make run MATRIX=Matrix/g7jac200.mtx

# Vuoi provare con -O0? DEVI usare build!
make build OPT=-O0
make run MATRIX=Matrix/g7jac200.mtx

# Se usi solo "make all OPT=-O0" senza clean:
# ⚠️  ERRORE: mix di -O3 e -O0!
La Differenza
Comando	Effetto
make all OPT=-O0	Compila solo file mancanti → SBAGLIATO se cambi OPT
make build OPT=-O0	Cancella tutto + ricompila → CORRETTO ✓
Usa build quando cambi l'ottimizzazione! 🎯

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
	@echo "" addattami questo makefile con questa accortezza del ottimizzazione

makefile
CC = gcc
# OPTIMIZATION: -O0 (none), -O1 (basic), -O2 (moderate), -O3 (aggressive)
OPT ?= -O3
CFLAGS = -Wall -g $(OPT) -fopenmp -std=c99 -I../Header
LIBS = -lm

# Parametri personalizzabili
MATRIX ?= ../Matrix/bcsstk13.mtx
THREADS ?= 4
SCHEDULE ?= 0
CHUNK ?= 1000

# Sorgenti
CSR_SEQ_SRC = csr_seq_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c
CSR_PAR_SRC = csr_par_bench.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c

.PHONY: all clean build perf_csr_seq perf_csr_par help

all: csr_seq_bench csr_par_bench

csr_seq_bench: $(CSR_SEQ_SRC)
	$(CC) $(CFLAGS) -o csr_seq_bench $(CSR_SEQ_SRC) $(LIBS)
	@echo "Compiled: csr_seq_bench with $(OPT)"

csr_par_bench: $(CSR_PAR_SRC)
	$(CC) $(CFLAGS) -o csr_par_bench $(CSR_PAR_SRC) $(LIBS)
	@echo "Compiled: csr_par_bench with $(OPT)"

build: clean all
	@echo "Clean rebuild completed with $(OPT)"

perf_csr_seq: all
	@echo "Running perf on SEQUENTIAL with $(OPT)..."
	@perf stat -e cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses \
	./csr_seq_bench "$(MATRIX)"

perf_csr_par: all
	@echo "Running perf on PARALLEL ($(THREADS) threads, schedule $(SCHEDULE), chunk $(CHUNK)) with $(OPT)..."
	@perf stat -e cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses \
	./csr_par_bench "$(MATRIX)" $(THREADS) $(SCHEDULE) $(CHUNK)

clean:
	rm -f csr_seq_bench csr_par_bench
	@echo "Cleaned"

help:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║              CSR SpMV Profiling with PERF                     ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "TARGETS:"
	@echo "  make build            - Ricompila da zero con nuove ottimizzazioni"
	@echo "  make all              - Compila (senza pulire)"
	@echo "  make perf_csr_seq     - CSR Sequenziale con perf"
	@echo "  make perf_csr_par     - CSR Parallelo con perf"
	@echo "  make clean            - Rimuove gli eseguibili"
	@echo ""
	@echo "VARIABILI CONFIGURABILI (default in parentesi):"
	@echo "  MATRIX   - Percorso della matrice sparse (../Matrix/bcsstk13.mtx)"
	@echo "  THREADS  - Numero di thread OpenMP (4)"
	@echo "  SCHEDULE - OpenMP schedule: 0=static, 1=dynamic, 2=guided (0=static)"
	@echo "  CHUNK    - Chunk size (default: 1000)"
	@echo "  OPT      - Ottimizzazione: -O0, -O1, -O2, -O3 (default: -O3)"
	@echo ""
	@echo "WORKFLOW CONSIGLIATO:"
	@echo ""
	@echo "  1. Compilazione iniziale:"
	@echo "     make build OPT=-O3"
	@echo ""
	@echo "  2. Profiling sequenziale:"
	@echo "     make perf_csr_seq MATRIX=../Matrix/bcsstk13.mtx"
	@echo ""
	@echo "  3. Profiling parallelo (8 thread, static, chunk 1000):"
	@echo "     make perf_csr_par MATRIX=../Matrix/bcsstk13.mtx THREADS=8 SCHEDULE=0 CHUNK=1000"
	@echo ""
	@echo "  4. Cambiare ottimizzazione:"
	@echo "     make build OPT=-O0"
	@echo "     make perf_csr_seq MATRIX=../Matrix/g7jac200.mtx"
	@echo ""
	@echo "ESEMPI DI UTILIZZO:"
	@echo ""
	@echo "  Compilazione con -O3:"
	@echo "    make build OPT=-O3"
	@echo ""
	@echo "  Compilazione con -O0 (no optimization):"
	@echo "    make build OPT=-O0"
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
Copia in Profiling/Makefile! 🎯









