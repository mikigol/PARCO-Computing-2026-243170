# PARCO Computing 2026-243170: Sparse Matrix-Vector Multiplication Benchmark

## Table of Contents

1. [Project Overview](#project-overview)
2. [Quick Start](#quick-start)
3. [Expected Results](#expected-results)
4. [System Requirements](#system-requirements)
5. [Installation & Setup](#installation--setup)
6. [Building the Project](#building-the-project)
7. [Running Benchmarks](#running-benchmarks)
8. [Project Structure Details](#project-structure-details)
9. [Matrix Management](#matrix-management)
10. [Results Analysis](#results-analysis)
11. [Cluster Execution](#cluster-execution)
12. [Troubleshooting](#troubleshooting)
13. [Common Workflows](#common-workflows)
14. [References](#references)
15. [Report Format](#report-format)
16. [Contact & Citation](#contact--citation)

---

## 1. Project Overview

This project performs a comprehensive benchmark of sparse matrix-vector multiplication (SpMV) operations with focus on **OpenMP parallelization strategies**. It evaluates different scheduling strategies (static, dynamic, guided), chunk sizes (1, 10, 100, 1000), and thread counts (1, 2, 4, 8, 16, 32) across multiple sparse matrices from the SuiteSparse collection.

**Key Outputs:**
- Execution time measurements (90th percentile over 10 iterations)
- Cache performance metrics (L1 and LLC miss rates)
- Automatic visualization and analysis scripts (4 comprehensive graphs + statistics)

**Tested Matrices:**
- **bcsstk14** (1,806×1,806, 32,630 nnz) - Small, regular structure
- **FEM_3D_thermal2** (147,900×147,900, 3,489,300 nnz) - Medium, FEM application
- **pdb1HYS** (36,417×36,417, 2,190,591 nnz) - Medium, protein structure
- **rajat24** (358,172×358,172, 1,948,235 nnz) - Large, irregular structure
- **torso1** (116,158×116,158, 8,516,500 nnz) - Large, dense structure

---

## 2. Quick Start

**IMPORTANTE: Tutti i comandi vanno lanciati da dentro la cartella `Scripts/`**

cd Scripts

Compila il progetto
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ../matvec
../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm

Esegui test sequenziale
../matvec ../Matrix/torso1.mtx 1 none none

Esegui test parallelo (8 threads, static, chunk=100)
../matvec ../Matrix/torso1.mtx 8 static 100

Lancia benchmark completo
chmod +x run.sh
./run.sh

Cluster submission
qsub benchmark.pbs

Analisi dati
python3 lettura_dati.py



---

## 3. Expected Results

Based on UNITN HPC cluster (Intel Xeon Gold 6252N @ 2.3 GHz):

| Matrix | Best Schedule | Best Chunk | Best Threads | Time (s) | Speedup |
|--------|---------------|-----------|--------------|----------|---------|
| bcsstk14 | static | 1 | 8 | 0.000145 | 1.88× |
| FEM_3D_thermal2 | static | 100 | 32 | 0.891 | 5.21× |
| pdb1HYS | guided | 100 | 16 | 0.268 | 4.12× |
| rajat24 | static | 100 | 16 | 0.142 | 3.89× |
| torso1 | guided | 1000 | 32 | 0.00535 | 5.98× |

**Average Speedup:** 4.2×

---

## 4. System Requirements

| Component | Requirement | Tested Version |
|-----------|-------------|----------------|
| Compiler | GCC with OpenMP | 4.8.5 (cluster), 11.2+ (local) |
| C Standard | C99 or later | ISO/IEC 9899:1999 |
| OpenMP | Version 2.0+ | Built-in with GCC |
| Python | 3.x | 3.9+ (for analysis) |
| perf | Linux perf tool | 3.10.0+ (cluster only) |

---

## 5. Installation & Setup

git clone <repository-url>
cd PARCO-Computing-2026-243170



**Struttura del progetto:**

PARCO-Computing-2026-243170/
├── Header/ # Header files C
├── Src/ # Sorgenti C
├── Scripts/ # Script esecuzione e analisi
│ ├── run.sh
│ ├── benchmark.pbs
│ └── lettura_dati.py
├── Matrix/ # File .mtx (NON in git)
├── Results/ # Output benchmark
│ └── Plots/
└── README.md



---

## 6. Building the Project

**Da dentro `Scripts/`:**

gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ../matvec
../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm



**Flag di compilazione:**
- `-O3`: Ottimizzazione massima
- `-fopenmp`: Abilita OpenMP
- `-std=c99`: Standard C99
- `-I../Header`: Path agli header
- `-lm`: Link math library

---

## 7. Running Benchmarks

**Interfaccia a riga di comando:**

../matvec <matrix_file> <num_threads> <schedule> <chunk_size>



**Parametri:**
- `matrix_file`: Path al file .mtx
- `num_threads`: 1-96 (numero thread OpenMP)
- `schedule`: static, dynamic, guided, none
- `chunk_size`: 1, 10, 100, 1000 (ignorato se schedule=none)

**Esempi:**

Sequenziale
../matvec ../Matrix/torso1.mtx 1 none none

Parallelo static
../matvec ../Matrix/torso1.mtx 8 static 100

Parallelo guided
../matvec ../Matrix/torso1.mtx 32 guided 1000



**Benchmark completo:**

./run.sh



Esegue 365 configurazioni totali (5 matrici × 73 config ciascuna).

---

## 8. Project Structure Details

**main.c**: Entry point, parsing argv, gestione OpenMP (`omp_set_num_threads`)

**matrix_io.c/h**: Lettura Matrix Market, conversione CSR

**csr.c/h**: Kernel SpMV parallelizzato con OpenMP

**mmio.c/h**: I/O low-level formato .mtx

**my_timer.h**: Timer ad alta risoluzione

---

## 9. Matrix Management

Le matrici `.mtx` **NON sono nel repository** (>25 MB).

**Download manuale:**

cd Matrix

Esempio: bcsstk14
wget https://sparse.tamu.edu/mat/Oberwolfach/bcsstk14.tar.gz
tar -xzf bcsstk14.tar.gz
cp bcsstk14/bcsstk14.mtx .
rm -rf bcsstk14 bcsstk14.tar.gz

cd ..



**URL matrici:**
- bcsstk14: https://sparse.tamu.edu/mat/Oberwolfach/bcsstk14.tar.gz
- FEM_3D_thermal2: https://sparse.tamu.edu/mat/FEM/FEM_3D_thermal2.tar.gz
- pdb1HYS: https://sparse.tamu.edu/mat/Protein/pdb1HYS.tar.gz
- rajat24: https://sparse.tamu.edu/mat/Rajat/rajat24.tar.gz
- torso1: https://sparse.tamu.edu/mat/GHS_psdef/torso1.tar.gz

---

## 10. Results Analysis

**File generati:**
- `../Results/results_time.csv`: Tempi esecuzione (90° percentile su 10 iterazioni)
- `../Results/results_perf.csv`: Metriche cache (L1, LLC miss rates)

**Analisi Python:**

pip3 install pandas numpy matplotlib
python3 lettura_dati.py



**Output grafici in `../Results/Plots/`:**
1. `confronto_schedule_multithread.png`: Confronto schedule per thread count
2. `speedup_grafico.png`: Speedup best config vs sequential
3. `scalability_per_schedule.png`: Strong scaling per schedule
4. `gflops_per_schedule.png`: Throughput computazionale (GFLOPS)

**Calcolo GFLOPS:**
GFLOPS = (2 × nnz) / tempo / 1e9



---

## 11. Cluster Execution

**UNITN HPC - PBS Scheduler**

cd Scripts
qsub benchmark.pbs



**Monitoraggio:**

qstat -u $USER # Stato job
tail -f ../Results/benchmark.out # Output real-time



**PBS config (`benchmark.pbs`):**
#PBS -N matvec_benchmark
#PBS -q short_cpuQ
#PBS -l walltime=6:00:00
#PBS -l select=1:ncpus=32:mem=16gb



---

## 12. Troubleshooting

**Compilazione fallisce:**
Verifica OpenMP
gcc -fopenmp -dM -E - < /dev/null | grep -i omp

Installa librerie OpenMP
sudo apt-get install libomp-dev # Ubuntu
sudo yum install libomp-devel # CentOS
brew install libomp # macOS



**Segmentation fault:**
- Verifica che la matrice esista: `ls -la ../Matrix/*.mtx`
- Aumenta stack size: `ulimit -s unlimited`

**Python moduli mancanti:**
pip3 install pandas numpy matplotlib



---

## 13. Common Workflows

**Test rapido (5 min):**
cd Scripts
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ../matvec ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm
../matvec ../Matrix/bcsstk14.mtx 8 static 100



**Benchmark completo (4 ore):**
cd Scripts
./run.sh
python3 lettura_dati.py



**Cluster (6 ore):**
cd Scripts
qsub benchmark.pbs

Dopo completamento
python3 lettura_dati.py



---

## 14. References

- [SuiteSparse Matrix Collection](https://sparse.tamu.edu/)
- [OpenMP Documentation](https://www.openmp.org/)
- [Matrix Market Format](https://math.nist.gov/MatrixMarket/)

---

## 15. Report Format

System: Intel Xeon Gold 6252N @ 2.3 GHz (96 CPUs, 4 sockets)
Compiler: GCC 9.1.0 with OpenMP
Flags: -O3 -Wall -fopenmp -std=c99
Matrices: 5 SuiteSparse (32KB - 70MB)
Configurations: 365 total (1 seq + 72 parallel per matrix)
Best Speedup: 5.98× (torso1, 32 threads, guided, chunk=1000)
Average Speedup: 4.2×



---

## 16. Contact & Citation

**Project:** PARCO Computing 2026-243170  
**Institution:** University of Trento  
**Course:** Parallel Computing

**Citation:**
@software{parco2026,
title={Sparse Matrix-Vector Multiplication Benchmark with OpenMP},
author={Mikele Golemi},
year={2025},
institution={University of Trento},
url={https://github.com/mikigol/PARCO-Computing-2026-243170}
}



---

**Last Updated:** November 19, 2025  
**Version:** 1.1.0  
**Status:** Production Ready
