# PARCO Computing 2026: Distributed Sparse Matrix-Vector Multiplication with MPI

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

---

## Project Overview

This project performs a comprehensive benchmark of **distributed sparse matrix-vector multiplication (SpMV)** using **MPI** (Message Passing Interface) with both **Pure MPI** and **Hybrid MPI+OpenMP** parallelization strategies. It evaluates strong scaling (fixed problem size, increasing processes) and weak scaling (problem size grows with processes) across multiple sparse matrices from the SuiteSparse collection.

**Key Features:**
- **Pure MPI implementation** with cyclic row distribution
- **Hybrid MPI+OpenMP** for multi-level parallelism
- **Ghost cell exchange** using MPI_Alltoallv for efficient communication
- **Strong scaling analysis** on real matrices (1 to 128 MPI processes)
- **Weak scaling analysis** with synthetic matrix generation (10,000 rows/process)
- **Comprehensive metrics**: execution time (P90), communication overhead, GFLOPS, load imbalance

**Tested Matrices:**
- **bcsstk14** (1,806×1,806, 32,630 nnz) - Small
- **pdb1HYS** (36,417×36,417, 2,190,591 nnz) - Medium
- **torso1** (116,158×116,158, 8,516,500 nnz) - Large

**Key Outputs:**
- Execution time measurements (90th percentile over 10 iterations)
- Communication time tracking
- Ghost cell exchange analysis
- Load imbalance metrics
- GFLOPS performance
- Automatic CSV generation and analysis scripts

---

## Quick Start

### For Local Testing (Single Matrix, Pure MPI)

```bash
# Navigate to scripts directory
cd scripts

# Compile Pure MPI version
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

# Run with 4 MPI processes
mpirun -np 4 ../results/spmv_mpi.out ../data/bcsstk14.mtx 10
```

### For Hybrid MPI+OpenMP Testing

```bash
# Compile Hybrid version
mpicc -O3 -Wall -lm -fopenmp -I../include -o ../results/spmv_hybrid.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

# Run with 4 MPI processes, 2 OpenMP threads each
export OMP_NUM_THREADS=2
mpirun -np 4 ../results/spmv_hybrid.out ../data/torso1.mtx 10
```

### For Full Benchmark on Local Machine

```bash
cd scripts
chmod +x test.sh
./test.sh
```

**Estimated runtime:** 3-6 hours (depending on CPU and available cores)

### For Cluster Submission (UNITN HPC)

```bash
cd scripts
qsub benchmark.pbs
```

**Estimated runtime:** 6 hours (6 nodes, 144 total MPI processes)

### Data Analysis

```bash
cd scripts
python3 extract_data.py
```

Summary tables will be generated in `results/tables/`

---

## Expected Results

Based on UNITN HPC cluster execution (Intel Xeon Gold 6140M @ 2.3 GHz, 6 nodes, 144 total cores):

### Strong Scaling - Pure MPI (Best Configurations)

| Matrix | Best Procs | P90 Time (s) | GFLOPS | Speedup vs 1 proc | Ghost Avg | Load Imbalance |
|--------|-----------|--------------|---------|-------------------|-----------|----------------|
| bcsstk14 | 8 | 0.000030 | 4.26 | 2.18× | 1,540 | 1.02 |
| pdb1HYS | 16 | 0.000877 | 9.91 | 5.84× | 34,137 | 1.01 |
| torso1 | 32 | 0.001195 | 14.25 | 10.06× | 19,371 | 1.02 |

### Strong Scaling - Performance Trends

**bcsstk14 (Small Matrix - 32K nnz):**
- Best performance: 8 processes (4.26 GFLOPS)
- Communication overhead dominates beyond 8 processes
- Speedup saturates at ~2× due to small problem size

**pdb1HYS (Medium Matrix - 2.2M nnz):**
- Best performance: 16 processes (9.91 GFLOPS)
- Good scaling efficiency up to 16 processes
- Ghost cell overhead increases significantly beyond 32 processes

**torso1 (Large Matrix - 8.5M nnz):**
- Best performance: 32 processes (14.25 GFLOPS)
- Excellent strong scaling up to 32 processes (10× speedup)
- Load imbalance remains low (1.02) even at high process counts
- Communication overhead manageable due to large computation/communication ratio

### Weak Scaling - Synthetic Matrices

**Configuration:** 10,000 rows/process, 50 nnz/row average
- Processes tested: 1, 2, 4, 8, 16, 32, 64, 128
- Total problem size scales linearly with process count
- Results available in `results/weak_scaling_all.csv`

**System:** Intel Xeon Gold 6140M @ 2.3 GHz (18 cores/socket, 4 sockets/node, 72 cores/node)

---

## System Requirements

### Compiler & Toolchain

| Component | Requirement | Tested Version |
|-----------|-------------|-----------------|
| MPI Library | MPICH or OpenMPI | MPICH 3.2.1 |
| Compiler | GCC with MPI support | GCC 9.1.0 |
| C Standard | C99 or later | ISO/IEC 9899:1999 |
| OpenMP | Version 2.0+ (for hybrid mode) | Built-in with GCC |
| Build Tool | Make or manual compilation | GNU Make 3.82+ |

### Hardware

**Minimum (Local):**
- CPU: Quad-core with MPI support
- RAM: 4 GB
- Storage: 500 MB (with matrices)
- Network: Not required for single-node testing

**Recommended (Cluster):**
- CPU: Intel Xeon Gold 6140M or equivalent (18+ cores per socket)
- RAM: 16 GB per node
- Network: High-speed interconnect (InfiniBand, 10+ Gbps Ethernet)
- NUMA: Multi-socket NUMA systems for optimal performance

**Tested Hardware (UNITN HPC):**
```
Architecture:  x86_64
CPU: Intel(R) Xeon(R) Gold 6140M @ 2.30GHz
Cores per socket: 18
Sockets per node: 4 (72 total cores per node)
Nodes tested: 6 (144 total MPI processes)
L1 cache: 32K (per core)
L2 cache: 1024K (per core)
L3 cache: 25344K (shared per socket)
Memory: 220 GB per node (4 NUMA nodes)
Kernel: Linux 3.10.0-1160.53.1.el7.x86_64
MPI: MPICH 3.2.1 with GCC 9.1.0
```

---

## Installation & Setup

### 1. Clone Repository

```bash
git clone <repository-url>
cd PARCO-MPI-SpMV-2026
```

### 2. Verify Directory Structure

The project structure should be:

```
PARCO-MPI-SpMV-2026/
├── include/              # C header files
│   ├── structures.h      # Data structures (LocalCSR, CommInfo)
│   ├── matrix_io.h       # Matrix I/O prototypes
│   └── mmio.h            # Matrix Market I/O
├── src/                  # C source files
│   ├── main.c            # Main entry point, benchmark loop
│   ├── io_setup.c        # Matrix loading and distribution
│   ├── computation.c     # SpMV kernel (with OpenMP)
│   ├── communication.c   # Ghost cell exchange (MPI_Alltoallv)
│   ├── matrix_io.c       # Matrix Market reader
│   └── mmio.c            # Matrix Market I/O library
├── scripts/              # Execution and analysis scripts
│   ├── test.sh           # Local/cluster benchmark runner
│   ├── benchmark.pbs     # PBS cluster submission script
│   ├── extract_data.py   # Results extraction and table generation
│   └── data.py           # Additional analysis utilities
├── data/                 # Sparse matrix files (.mtx) - NOT in git
│   └── info.txt          # Instructions for matrix setup
├── results/              # Benchmark output directory (generated)
│   ├── tables/           # Summary tables (CSV)
│   ├── logs/             # Individual run logs
│   ├── strong_scaling_all.csv       # Pure MPI strong scaling
│   ├── strong_scaling_hybrid.csv    # Hybrid MPI+OMP strong scaling
│   ├── weak_scaling_all.csv         # Pure MPI weak scaling
│   └── weak_scaling_hybrid.csv      # Hybrid MPI+OMP weak scaling
└── README.md             # This file
```

### 3. Verify MPI Installation

**For local systems:**
```bash
# Check MPI compiler
mpicc --version
which mpirun

# Test MPI
mpirun -np 2 hostname
```

**For UNITN HPC cluster:**
```bash
# Load required modules
module load gcc91
module load mpich-3.2.1--gcc-9.1.0

# Verify
mpicc --version  # Should show GCC 9.1.0
which mpirun     # Should point to MPICH installation
```

---

## Building the Project

### Pure MPI Build

Navigate to the `scripts/` directory first:

```bash
cd scripts
```

Then compile:

```bash
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c
```

**Compilation Flags Explanation:**
- `-O3`: Maximum optimization level
- `-Wall`: Enable all warnings
- `-lm`: Link math library
- `-I../include`: Include path for header files
- `-o ../results/spmv_mpi.out`: Output executable path

### Hybrid MPI+OpenMP Build

```bash
cd scripts

mpicc -O3 -Wall -lm -fopenmp -I../include -o ../results/spmv_hybrid.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c
```

**Additional flag:**
- `-fopenmp`: Enable OpenMP support for hybrid parallelism

### Automated Build (via test.sh)

The `test.sh` script automatically compiles both versions before running benchmarks:

```bash
cd scripts
chmod +x test.sh
./test.sh  # Compiles and runs benchmarks
```

### Troubleshooting Build Issues

If compilation fails:

```bash
# Check for MPI headers
find /usr -name mpi.h 2>/dev/null

# Verify MPI compiler wrapper
mpicc -show  # Shows underlying gcc command

# Try verbose compilation
mpicc -v -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c
```

---

## Running Benchmarks

### Command-Line Interface

The program accepts the following arguments:

#### For Strong Scaling (Real Matrices)
```bash
mpirun -np <num_processes> <executable> <matrix_file> <repeats>
```

#### For Weak Scaling (Synthetic Matrices)
```bash
mpirun -np <num_processes> <executable> synthetic <repeats> <rows_per_proc> <nnz_per_row>
```

**Parameters:**

| Parameter | Type | Values | Default | Meaning |
|-----------|------|--------|---------|---------|
| `num_processes` | int | 1-128 | - | Number of MPI processes |
| `matrix_file` | string | Path to `.mtx` file | - | Sparse matrix in Matrix Market format |
| `repeats` | int | 1-100 | 10 | Number of iterations per benchmark |
| `rows_per_proc` | int | 100-100000 | 10000 | Rows per process (weak scaling) |
| `nnz_per_row` | int | 1-1000 | 50 | Average non-zeros per row (weak scaling) |

### Examples

#### Strong Scaling - Pure MPI

```bash
cd scripts

# Single process (baseline)
mpirun -np 1 ../results/spmv_mpi.out ../data/bcsstk14.mtx 10

# 4 processes
mpirun -np 4 ../results/spmv_mpi.out ../data/torso1.mtx 10

# 16 processes
mpirun -np 16 ../results/spmv_mpi.out ../data/pdb1HYS.mtx 10
```

**Output format (per process per iteration):**
```csv
matrix_name,rank,num_procs,run,elapsed_time,comm_time,local_nz,ghost_entries,local_flops
../data/torso1.mtx,0,4,0,0.003632784,0.000245123,2129125,79542,4258250
```

Followed by summary metrics:
```csv
Matrix_Name,Num_Processes,Ghost_Entries_Min,Ghost_Entries_Avg,Ghost_Entries_Max,Load_Imbalance_Ratio,System_P90_Time,Total_GFLOPs
../data/torso1.mtx,4,79423,79482.50,79542,1.0014,0.003632784,4.6887
```

#### Strong Scaling - Hybrid MPI+OpenMP

```bash
cd scripts

# 4 MPI processes, 4 OpenMP threads each (16 total threads)
export OMP_NUM_THREADS=4
export OMP_SCHEDULE=dynamic,64
export OMP_PROC_BIND=close
mpirun -np 4 ../results/spmv_hybrid.out ../data/torso1.mtx 10

# 8 MPI processes, 2 OpenMP threads each (16 total threads)
export OMP_NUM_THREADS=2
mpirun -np 8 ../results/spmv_hybrid.out ../data/pdb1HYS.mtx 10
```

#### Weak Scaling - Synthetic Matrices

```bash
cd scripts

# 1 process: 10,000 rows, 50 nnz/row
mpirun -np 1 ../results/spmv_mpi.out synthetic 10 10000 50

# 16 processes: 10,000 rows/process (160,000 total rows)
mpirun -np 16 ../results/spmv_mpi.out synthetic 10 10000 50

# 128 processes: 10,000 rows/process (1,280,000 total rows)
mpirun -np 128 ../results/spmv_mpi.out synthetic 10 10000 50
```

### Full Automated Benchmark

Run all combinations automatically using the test script:

```bash
cd scripts
chmod +x test.sh
./test.sh
```

**What this does:**

**Strong Scaling (Real Matrices):**
- 3 matrices: bcsstk14, pdb1HYS, torso1
- Process counts: 1, 2, 4, 8, 16, 32, 64, 128
- Both Pure MPI and Hybrid modes
- 10 iterations per configuration
- **Subtotal:** 3 matrices × 8 process counts × 2 modes = 48 configurations

**Weak Scaling (Synthetic):**
- Process counts: 1, 2, 4, 8, 16, 32, 64, 128
- Both Pure MPI and Hybrid modes
- 10 iterations per configuration
- **Subtotal:** 8 process counts × 2 modes = 16 configurations

**Total:** 64 benchmark configurations × 10 iterations = 640 runs

**Outputs:**
- `results/strong_scaling_all.csv` - Pure MPI strong scaling results
- `results/strong_scaling_hybrid.csv` - Hybrid MPI+OMP strong scaling results
- `results/weak_scaling_all.csv` - Pure MPI weak scaling results
- `results/weak_scaling_hybrid.csv` - Hybrid MPI+OMP weak scaling results
- `results/logs/` - Individual run logs

**Estimated runtime:**
- Local (8-16 cores): 3-6 hours
- Cluster (144 cores): ~2-3 hours

### Default Benchmark Parameters (from test.sh)

```bash
# Process counts tested
PROCESSES=(1 2 4 8 16 32 64 128)

# MPI configuration
PROCESSES_PER_NODE=24  # Adjust based on your system
MAX_PROCESSES=128

# OpenMP configuration (for Hybrid mode)
OMP_SCHEDULE="dynamic,64"
OMP_PROC_BIND="close"

# Weak scaling parameters
REPEATS=10
ROWS_PER_PROC=10000
NNZ_PER_ROW=50
```

**To customize parameters**, edit `scripts/test.sh` before running.

---

## Project Structure Details

### Source Files

**main.c** - Main entry point and benchmark orchestration
- Command-line argument parsing (matrix file vs synthetic, repeats, parameters)
- Timing measurements: 10 iterations, reports 90th percentile
- MPI initialization with thread support (`MPI_Init_thread`)
- Coordinates matrix loading, communication setup, and computation
- CSV output generation with detailed metrics
- Summary statistics computation (GFLOPS, load imbalance, ghost cells)

**io_setup.c** - Matrix distribution and CSR conversion
- `load_and_scatter_matrix()`: Rank 0 reads .mtx file, distributes to all processes
- Cyclic row distribution using `GET_OWNER(row, size)` macro
- COO to CSR format conversion
- Memory-efficient scatter pattern (avoids broadcasting entire matrix)
- MPI point-to-point communication for distribution

**communication.c** - Ghost cell exchange implementation
- `setup_communication_pattern()`: One-time setup of communication structure
  - Identifies which columns are ghost cells (owned by other processes)
  - Builds send/receive counts and displacement arrays for `MPI_Alltoallv`
  - Renumbers column indices (local + ghost regions)
- `perform_ghost_exchange()`: Runtime ghost cell exchange
  - Pack phase: copy local data to send buffer (OpenMP parallelized)
  - MPI_Alltoallv: all-to-all communication
  - Unpack phase: copy received data to ghost region (OpenMP parallelized)
- `generate_synthetic_matrix()`: Creates random sparse matrices for weak scaling
  - Block distribution (deterministic based on rank)
  - Configurable rows/process and nnz/row
  - Randomized column indices with duplicate checking

**computation.c** - CSR matrix-vector multiplication kernel
- `compute_spmv()`: Core SpMV operation
  - Row-wise parallel loop (OpenMP with runtime scheduling)
  - Vectorizable inner loop for each row
  - Reads from both local and ghost regions of x vector
  - Writes results to local y vector

**matrix_io.c / matrix_io.h** - Matrix Market file I/O
- Matrix Market format reading (.mtx files)
- Handles symmetric matrices (expands to full format)
- Pattern matrices (assigns value = 1.0)
- COO format storage (I, J, val arrays)
- Memory allocation and deallocation

**mmio.c / mmio.h** - Matrix Market I/O library (Reference Implementation)
- Low-level .mtx file parsing
- Banner and metadata reading
- Type code handling (real, complex, pattern, symmetric)
- Reference implementation from SuiteSparse

### Header Files

- `structures.h` - Core data structures and distribution macros
  - `LocalCSR`: CSR matrix storage (row_ptr, col_ind, val)
  - `CommInfo`: Ghost exchange metadata (counts, displacements, buffers)
  - `GET_OWNER(idx, size)`: Cyclic distribution owner calculation
  - `GET_LOCAL_IDX(idx, size)`: Global to local index mapping

- `matrix_io.h` - Matrix I/O function prototypes
  - `read_matrix()`: Load .mtx file
  - `free_matrix()`: Deallocate matrix

- `mmio.h` - Matrix Market I/O routines
  - Banner parsing
  - Type code utilities
  - Error codes

### Key Algorithmic Details

**Cyclic Row Distribution:**
```c
#define GET_OWNER(glob_idx, size) ((glob_idx) % (size))
#define GET_LOCAL_IDX(glob_idx, size) ((glob_idx) / (size))
```
- Row `i` is owned by process `i % num_processes`
- Better load balance for irregular matrices than block distribution
- Local index: `i / num_processes` (0-indexed on each process)

**Ghost Cell Communication Pattern:**
1. **Setup phase** (once per matrix):
   - Scan local CSR to find non-local columns
   - Build request lists per owning process
   - Exchange requests using MPI_Alltoall + MPI_Alltoallv
   - Renumber column indices (local: [0, n_local), ghost: [n_local, n_local+n_ghost))

2. **Runtime phase** (every SpMV iteration):
   - Pack: Copy requested local data to send buffer
   - MPI_Alltoallv: Exchange ghost values
   - Unpack: Copy received data to ghost region of x vector

**Hybrid Parallelism:**
- MPI: Distributed memory parallelism across nodes/processes
- OpenMP: Shared memory parallelism within each MPI process
  - Used in ghost cell pack/unpack loops
  - Used in SpMV computation loop
  - Dynamic scheduling for load balancing

---

## Matrix Management

### Automatic Download Setup

The `data/` directory should contain the `.mtx` files. Due to size constraints (>25 MB per file), matrices are **not included in the Git repository**.

### Downloading Matrices

#### Option 1: Manual Download (Recommended)

1. Visit [https://sparse.tamu.edu/](https://sparse.tamu.edu/)
2. Search for each matrix name individually
3. Download the `.tar.gz` file
4. Extract and locate the `.mtx` file
5. Copy to `data/` directory

**Step-by-step example for torso1:**

```bash
cd data/

# Download
wget https://sparse.tamu.edu/Norris/torso1.tar.gz

# Extract
tar -xzf torso1.tar.gz

# Copy the .mtx file
cp torso1/torso1.mtx .

# Cleanup
rm -rf torso1 torso1.tar.gz

cd ..
```

**Matrix URLs for Reference:**
- bcsstk14: https://sparse.tamu.edu/HB/bcsstk14
- pdb1HYS: https://sparse.tamu.edu/Williams/pdb1HYS
- torso1: https://sparse.tamu.edu/Norris/torso1

### Verify Matrix Files

```bash
# Check matrices are present
ls -lh data/*.mtx

# Expected output (sizes may vary slightly):
# -rw-r--r--  0.8M  bcsstk14.mtx
# -rw-r--r-- 17.9M  pdb1HYS.mtx
# -rw-r--r-- 70.2M  torso1.mtx
```

### Matrix Details

| Matrix | Rows | Cols | NNZ | NNZ (symmetric expansion) | Source | Category |
|--------|------|------|-----|---------------------------|--------|----------|
| bcsstk14 | 1,806 | 1,806 | 32,630 | 63,454 | HB | Structural |
| pdb1HYS | 36,417 | 36,417 | 2,190,591 | 4,344,765 | Williams | Protein |
| torso1 | 116,158 | 116,158 | 8,516,500 | 17,033,000 | Norris | Medical |

**Note:** Symmetric matrices are expanded to full format (both (i,j) and (j,i) stored)

---

## Results Analysis

### Data Files Generated

After running benchmarks, four CSV files are created in `results/`:

#### 1. `strong_scaling_all.csv` - Pure MPI Strong Scaling

**Schema:**
```csv
matrix_name,rank,num_procs,run,elapsed_time,comm_time,local_nz,ghost_entries,local_flops
```

**Fields:**
- `matrix_name`: Matrix filename (e.g., "../data/torso1.mtx")
- `rank`: MPI process rank (0 to num_procs-1)
- `num_procs`: Total number of MPI processes
- `run`: Iteration number (0 to 9)
- `elapsed_time`: Total execution time in seconds (including communication)
- `comm_time`: Communication time in seconds (ghost exchange)
- `local_nz`: Number of non-zeros owned by this process
- `ghost_entries`: Number of ghost cells needed by this process
- `local_flops`: Floating-point operations (2 × local_nz)

**Summary Metrics (appended after each process count):**
```csv
Matrix_Name,Num_Processes,Ghost_Entries_Min,Ghost_Entries_Avg,Ghost_Entries_Max,Load_Imbalance_Ratio,System_P90_Time,Total_GFLOPs
```

**Example rows:**
```csv
../data/torso1.mtx,0,4,0,0.003632784,0.000245123,2129125,79542,4258250
../data/torso1.mtx,1,4,0,0.003601074,0.000223160,2129375,79423,4258750
Matrix_Name,Num_Processes,Ghost_Entries_Min,Ghost_Entries_Avg,Ghost_Entries_Max,Load_Imbalance_Ratio,System_P90_Time,Total_GFLOPs
../data/torso1.mtx,4,79423,79482.50,79542,1.0014,0.003632784,4.6887
```

#### 2. `strong_scaling_hybrid.csv` - Hybrid MPI+OpenMP Strong Scaling

Same schema as `strong_scaling_all.csv`, but with hybrid parallelism.

**OpenMP configuration (per test.sh defaults):**
- Threads per MPI process: `PROCESSES_PER_NODE / num_procs`
- Schedule: `dynamic,64`
- Binding: `close` (thread affinity)

#### 3. `weak_scaling_all.csv` - Pure MPI Weak Scaling

Same schema, but `matrix_name` is `synthetic_npN` where N is the number of processes.

**Example:**
```csv
synthetic_np16,0,16,0,0.002145678,0.000123456,200000,5234,400000
```

#### 4. `weak_scaling_hybrid.csv` - Hybrid MPI+OpenMP Weak Scaling

Same as weak_scaling_all.csv with hybrid parallelism.

### Python Analysis Script

Extract summary metrics and generate CSV tables:

```bash
cd scripts
python3 extract_data.py
```

**Prerequisites:**
```bash
pip3 install pandas
# or
conda install pandas
```

**What it does:**
1. Parses all four result CSV files
2. Extracts summary metrics (GFLOPS, P90 time, load imbalance, ghost cells)
3. Generates separate summary tables in `results/tables/`:
   - `summary_bcsstk14.csv` - bcsstk14 strong scaling summary
   - `summary_pdb1HYS.csv` - pdb1HYS strong scaling summary
   - `summary_torso1.csv` - torso1 strong scaling summary
   - `summary_synthetic_combined.csv` - Weak scaling summary (all process counts)

**Output columns:**
```csv
matrix_name,mode,num_procs,total_gflops,p90_time,load_imbalance,ghost_avg
```

**Example output (`results/tables/summary_torso1.csv`):**
```csv
matrix_name,mode,num_procs,total_gflops,p90_time,load_imbalance,ghost_avg
torso1.mtx,Pure MPI,1,1.4161,0.012027979,1.0000,0.00
torso1.mtx,Pure MPI,4,4.6887,0.003632784,1.0014,79482.50
torso1.mtx,Pure MPI,16,4.1715,0.004083157,1.0095,30817.81
torso1.mtx,Pure MPI,32,14.2513,0.001195192,1.0182,19371.28
torso1.mtx,Hybrid (MPI+OMP),1,1.4523,0.011728764,1.0000,0.00
torso1.mtx,Hybrid (MPI+OMP),4,5.1234,0.003321456,1.0012,79456.25
```

### Key Metrics Explained

**GFLOPS (Giga Floating-Point Operations Per Second):**
```
GFLOPS = (2 × total_nnz) / P90_time / 1e9
```
Where `total_nnz` is the total number of non-zeros across all processes.

**P90 Time:**
- 90th percentile of execution times across all iterations (per process)
- System P90: Maximum P90 time across all processes (system bottleneck)

**Load Imbalance Ratio:**
```
Load_Imbalance = max(local_nz) / avg(local_nz)
```
- Perfect balance: 1.0
- Higher values indicate uneven work distribution
- Cyclic distribution generally provides better balance than block distribution

**Ghost Cells:**
- Non-local column indices needed for local computation
- Ghost_Avg: Average ghost cells per process
- Higher ghost counts → more communication overhead

**Communication Overhead:**
```
Comm_Overhead = comm_time / elapsed_time
```
- Typical values: 5-20% for strong scaling
- Increases with process count (more boundaries)

---

## Cluster Execution

### UNITN HPC System Details

**Cluster:** UNITN HPC  
**Scheduler:** PBS (Portable Batch System)  
**Nodes:** CPU-only compute nodes  
**Node Configuration:**
- CPU: Intel Xeon Gold 6140M @ 2.3 GHz
- Cores per node: 72 (18 cores/socket × 4 sockets)
- Memory: 220 GB per node
- NUMA nodes: 4 per node

**Login:** `ssh username@hpc.unitn.it`

### Manual Execution (Interactive Session)

To run your code manually on the UNITN HPC cluster:

#### 1. Load Required Modules

After logging in, load the necessary environment modules:
```bash
module load gcc91
module load mpich-3.2.1--gcc-9.1.0
```

#### 2. Create an Interactive Session

Request an interactive session using PBS:
```bash
qsub -I -q short_cpuQ -l select=1:ncpus=24:mpiprocs=24:mem=4gb -l walltime=01:00:00
```

**Parameters:**
- `-I`: Interactive mode
- `-q short_cpuQ`: Queue name (max 6 hours)
- `select=1`: Number of nodes
- `ncpus=24`: CPUs per node
- `mpiprocs=24`: MPI processes per node
- `mem=4gb`: Memory per node
- `walltime=01:00:00`: Time limit (HH:MM:SS)

#### 3. Navigate and Compile

Once in the interactive session:
```bash
cd $PBS_O_WORKDIR  # Go to submission directory
cd scripts

# Compile (same as local)
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c
```

#### 4. Run Test

```bash
# Test with 4 processes
mpirun -np 4 ../results/spmv_mpi.out ../data/bcsstk14.mtx 10
```

### Full Batch Submission

For comprehensive benchmarks, submit the PBS script:

```bash
cd scripts
qsub benchmark.pbs
```

**Check job status:**
```bash
qstat -u $USER
```

**Check all jobs in queue:**
```bash
qstat
```

**Cancel job:**
```bash
qdel <job_id>
```

**Monitor output in real-time:**
```bash
tail -f ../results/job_output.out
```

### PBS Script Breakdown (benchmark.pbs)

The script performs the following:

```bash
#!/bin/bash
#PBS -N spmv_benchmark           # Job name
#PBS -o ../results/job_output.out  # Stdout file
#PBS -e ../results/job_error.err   # Stderr file
#PBS -q short_cpuQ                # Queue (6 hours max)
#PBS -l walltime=06:00:00         # Walltime limit
#PBS -l select=6:ncpus=24:mpiprocs=24:mem=4gb  # 6 nodes, 24 procs/node, 4GB/node
```

**Key execution phases:**

1. **Module Loading**
   ```bash
   module load gcc91
   module load mpich-3.2.1--gcc-9.1.0
   ```

2. **System Information Collection** → `system_info_<jobid>.txt`
   - OS, kernel, CPU details
   - Compiler and toolchain versions
   - Node allocation
   - Cache hierarchy
   - Memory configuration

3. **Environment Verification**
   - Checks for required directories (src, include, data)
   - Validates test.sh script exists
   - Sets executable permissions

4. **MPI Environment Configuration**
   ```bash
   export MPICH_PROCESS_AFFINITY=1  # Enable process affinity
   export MPICH_CPU_BINDING=1       # Enable CPU binding
   ```

5. **Benchmark Execution**
   - Runs `test.sh` script
   - Compiles both Pure MPI and Hybrid versions
   - Executes strong scaling (3 matrices × 8 process counts × 2 modes)
   - Executes weak scaling (8 process counts × 2 modes)
   - Generates CSV output files

6. **Results Summary**
   - Counts total runs in each CSV file
   - Reports completion status
   - Archives system information

### Output Files

After submission, check results:

```bash
# Monitor progress in real-time
tail -f results/job_output.out

# After completion, check for errors
cat results/job_error.err

# Verify data files were generated
ls -lh results/*.csv

# Check system info
cat results/system_info_*.txt

# View individual run logs
ls -lh results/logs/
```

### Customizing PBS Parameters

Edit `scripts/benchmark.pbs`:

```bash
# Change job name
#PBS -N my_custom_job_name

# Increase walltime (if needed)
#PBS -l walltime=12:00:00

# Use different queue (long = 72 hours)
#PBS -q long_cpuQ

# Request more nodes (for higher process counts)
#PBS -l select=12:ncpus=24:mpiprocs=24:mem=4gb  # 288 total processes

# Request more memory per node
#PBS -l select=6:ncpus=24:mpiprocs=24:mem=16gb
```

### Performance Notes for Cluster

1. **Process Placement:**
   - MPICH automatically handles process placement across nodes
   - `MPICH_PROCESS_AFFINITY=1` binds processes to cores
   - For 144 processes on 6 nodes: 24 processes per node

2. **NUMA Awareness:**
   - Each node has 4 NUMA domains (1 per socket)
   - Processes should be placed with NUMA locality in mind
   - L3 cache (25 MB) is shared per socket (18 cores)

3. **Network Communication:**
   - Ghost cell exchange uses MPI_Alltoallv
   - Communication pattern is all-to-all
   - Network bandwidth critical for high process counts

4. **Hybrid Parallelism:**
   - When using Hybrid mode, balance MPI processes vs OpenMP threads
   - Example: 72 processes/node (no OpenMP) vs 24 processes/node with 3 threads each
   - Generally, more MPI processes better for communication-bound problems

5. **Scaling Limits:**
   - bcsstk14 (small): Poor scaling beyond 8 processes (communication-dominated)
   - torso1 (large): Good scaling up to 32-64 processes
   - Weak scaling: Near-linear expected (constant work per process)

---

## Troubleshooting

### Compilation Issues

**Error: `mpicc: command not found`**
```bash
# Check if MPI is installed
which mpicc

# On cluster, load MPI module
module load mpich-3.2.1--gcc-9.1.0

# Verify
mpicc --version
```

**Error: `fatal error: mpi.h: No such file or directory`**
```bash
# Find MPI headers
find /usr -name mpi.h 2>/dev/null

# Explicitly specify include path
mpicc -O3 -I/path/to/mpi/include -o spmv_mpi.out ...
```

**Error: `undefined reference to 'omp_get_thread_num'`**
```bash
# Missing -fopenmp flag
mpicc -O3 -Wall -lm -fopenmp -I../include -o ../results/spmv_hybrid.out ...
```

### Runtime Issues

**Error: `Segmentation fault`**
- Check matrix file exists and is readable: `ls -la data/*.mtx`
- Verify matrix format is valid Matrix Market: `head -10 data/bcsstk14.mtx`
- Try smaller matrix first: `mpirun -np 2 spmv_mpi.out data/bcsstk14.mtx 1`

**Error: `MPI_Abort`**
- Check MPI process count matches available cores
- Review error message in stdout/stderr
- Run with verbose MPI: `mpirun -v -np 4 ...`

**Warning: `Ghost cells exceed expected count`**
- Normal for highly irregular matrices
- May indicate memory allocation issue
- Check `num_ghosts` in output matches `Ghost_Entries_Avg`

### Cluster Issues

**Error: `Job rejected: Walltime exceeds limit`**
- Reduce walltime: `#PBS -l walltime=03:00:00`
- Or use long queue: `#PBS -q long_cpuQ` (72 hours max)

**Error: `Out of memory`**
- Request more memory: `#PBS -l select=6:ncpus=24:mpiprocs=24:mem=16gb`
- Or reduce process count per node: `mpiprocs=12`

**Job not starting:**
```bash
# Check queue status
qstat -q

# Check node availability
pbsnodes -a | grep -A 3 "free"

# View job details
qstat -f <job_id>

# Submit to different queue
#PBS -q short_cpuQ  # vs long_cpuQ
```

**Job killed unexpectedly:**
```bash
# Check error file
cat results/job_error.err

# Check system logs
qstat -x <job_id>  # Shows exit status

# Common causes:
# - Exceeded walltime
# - Exceeded memory
# - Node failure
```

### Data Analysis Issues

**Python module not found:**
```bash
pip3 install pandas
# or
conda install pandas
```

**CSV file not found:**
```bash
# Verify benchmark completed successfully
ls results/*.csv

# If missing, check stderr
cat results/job_error.err

# Check individual logs
ls results/logs/
```

**Empty or corrupted CSV:**
```bash
# Verify file size
ls -lh results/strong_scaling_all.csv

# Check for header
head -1 results/strong_scaling_all.csv

# Count data rows (excluding headers and blanks)
grep -v "^$" results/strong_scaling_all.csv | grep -v "Matrix_Name" | wc -l
```

### Performance Issues

**Poor scaling:**
- Check load imbalance ratio in summary metrics
- Verify communication overhead is reasonable (<30%)
- Try different process counts (powers of 2 often work best)
- For small matrices, use fewer processes (communication overhead dominates)

**Slow execution:**
- Verify optimization flags: `-O3`
- Check if running on compute nodes (not login node)
- Monitor CPU usage: `top` or `htop`
- Check for I/O bottlenecks (shared filesystem)

---

## Common Workflows

### Workflow 1: Quick Local Test (5 minutes)

Perfect for verifying everything works:

```bash
cd scripts

# Compile Pure MPI
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

# Test single configuration (4 processes, small matrix)
mpirun -np 4 ../results/spmv_mpi.out ../data/bcsstk14.mtx 3

# Expected output: CSV data + summary metrics
```

### Workflow 2: Full Local Benchmark (3-6 hours)

Complete benchmark of all matrices and configurations:

```bash
cd scripts

# Make script executable
chmod +x test.sh

# Run full benchmark (compiles automatically)
./test.sh

# Monitor progress (in another terminal)
tail -f ../results/logs/strong_MPI_bcsstk14_np4.log

# After completion, extract summary tables
python3 extract_data.py

# View results
ls -lh ../results/tables/
cat ../results/tables/summary_torso1.csv
```

### Workflow 3: Cluster Submission (6 hours)

Submit to UNITN HPC for full execution:

```bash
# SSH to cluster
ssh username@hpc.unitn.it

# Navigate to project
cd /path/to/PARCO-MPI-SpMV-2026/scripts

# Load modules
module load gcc91
module load mpich-3.2.1--gcc-9.1.0

# Submit job
qsub benchmark.pbs

# Get job ID from output (e.g., 5038560.hpc-head-n1.unitn.it)
# Monitor job
qstat -u $USER

# Watch progress (updates every 10 sec)
watch -n 10 qstat -u $USER

# Monitor output in real-time
tail -f ../results/job_output.out

# After completion, download results
# (On local machine)
scp -r username@hpc.unitn.it:/path/to/PARCO-MPI-SpMV-2026/results ~/Downloads/
```

### Workflow 4: Single Matrix Strong Scaling Analysis

Focus on one matrix with different process counts:

```bash
cd scripts

# Compile
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

MATRIX="../data/torso1.mtx"
REPEATS=10

# Test different process counts
for np in 1 2 4 8 16 32; do
    echo "Running with $np processes..."
    mpirun -np $np ../results/spmv_mpi.out $MATRIX $REPEATS > \
        ../results/logs/torso1_np${np}.log 2>&1
done

# Extract summary metrics
grep "System_P90_Time" ../results/logs/torso1_np*.log
```

### Workflow 5: Weak Scaling Experiment

Test scalability with constant work per process:

```bash
cd scripts

# Compile
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

ROWS_PER_PROC=10000
NNZ_PER_ROW=50
REPEATS=10

# Test different process counts (problem size scales)
for np in 1 2 4 8 16 32 64; do
    echo "Running weak scaling with $np processes..."
    mpirun -np $np ../results/spmv_mpi.out synthetic $REPEATS \
        $ROWS_PER_PROC $NNZ_PER_ROW > ../results/logs/weak_np${np}.log 2>&1
done

# Check efficiency (should be near-constant time)
grep "System_P90_Time" ../results/logs/weak_np*.log
```

### Workflow 6: Hybrid MPI+OpenMP Comparison

Compare Pure MPI vs Hybrid performance:

```bash
cd scripts

# Compile both versions
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

mpicc -O3 -Wall -lm -fopenmp -I../include -o ../results/spmv_hybrid.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

MATRIX="../data/torso1.mtx"
REPEATS=10
TOTAL_THREADS=16  # Total parallelism

# Pure MPI: 16 processes
echo "Pure MPI (16 processes):"
mpirun -np 16 ../results/spmv_mpi.out $MATRIX $REPEATS | grep "System_P90_Time"

# Hybrid: 8 MPI × 2 OpenMP = 16 threads
echo "Hybrid (8 MPI × 2 OMP):"
export OMP_NUM_THREADS=2
mpirun -np 8 ../results/spmv_hybrid.out $MATRIX $REPEATS | grep "System_P90_Time"

# Hybrid: 4 MPI × 4 OpenMP = 16 threads
echo "Hybrid (4 MPI × 4 OMP):"
export OMP_NUM_THREADS=4
mpirun -np 4 ../results/spmv_hybrid.out $MATRIX $REPEATS | grep "System_P90_Time"
```

---

## References

**Resources & Documentation:**
- [SuiteSparse Matrix Collection](https://sparse.tamu.edu/)
- [Matrix Market Format Specification](https://math.nist.gov/MatrixMarket/)
- [MPI Standard Documentation](https://www.mpi-forum.org/docs/)
- [OpenMP Official Documentation](https://www.openmp.org/)
- [MPICH Documentation](https://www.mpich.org/documentation/)

**Key Papers:**
- Bell, N., & Garland, M. (2009). "Implementing sparse matrix-vector multiplication on throughput-oriented processors." Proceedings of SC'09.
- Williams, S., et al. (2007). "Optimization of sparse matrix-vector multiplication on emerging multicore platforms." Proceedings of SC'07.

---

## Report Format

When sharing results, include:

```markdown
## MPI SpMV Benchmark Results - PARCO Computing 2026

**System Information:**
- OS: Linux 3.10.0-1160.53.1.el7.x86_64
- CPU: Intel Xeon Gold 6140M @ 2.3 GHz (18 cores/socket, 4 sockets/node)
- Nodes: 6 compute nodes (72 cores/node, 220 GB RAM/node)
- Total MPI processes: 144 (24 processes/node)
- MPI: MPICH 3.2.1
- Compiler: GCC 9.1.0
- Compilation Flags: -O3 -Wall -lm -I../include

**Benchmark Configuration:**
- Matrices: 3 SuiteSparse matrices (bcsstk14, pdb1HYS, torso1)
- Process counts: 1, 2, 4, 8, 16, 32, 64, 128
- Modes: Pure MPI, Hybrid MPI+OpenMP
- Iterations: 10 per configuration (P90 reported)
- Distribution: Cyclic row distribution
- Communication: MPI_Alltoallv for ghost cell exchange

**Strong Scaling Results (Pure MPI):**
- bcsstk14: Best = 8 procs (4.26 GFLOPS, 2.18× speedup)
- pdb1HYS: Best = 16 procs (9.91 GFLOPS, 5.84× speedup)
- torso1: Best = 32 procs (14.25 GFLOPS, 10.06× speedup)

**Weak Scaling Results:**
- Configuration: 10,000 rows/process, 50 nnz/row
- Processes: 1-128
- Results: See weak_scaling_all.csv

**Key Findings:**
- Large matrices (torso1) scale well up to 32 processes
- Small matrices (bcsstk14) limited by communication overhead
- Cyclic distribution provides good load balance (1.01-1.04)
- Ghost cell overhead manageable for large matrices
- Hybrid mode performance varies by problem size

**Execution Date:** [DATE]
**Total Runtime:** [TIME]
```

---

## Contact & Information

**Project:** PARCO Computing 2026  
**Institution:** University of Trento  
**Course:** Parallel Computing

For questions or issues:
1. Check the troubleshooting section above
2. Review `results/system_info_*.txt` for environment details
3. Check `results/job_error.err` for error messages
4. Verify matrices are downloaded and readable
5. Ensure MPI modules are loaded correctly

---

## License & Citation

If using this benchmark in research, please cite:

```bibtex
@software{parco_mpi_spmv_2026,
  title={Distributed Sparse Matrix-Vector Multiplication with MPI},
  author={[Student Name]},
  year={2026},
  institution={University of Trento},
  url={[GitHub URL]}
}
```

**Related Work:**
- Bell & Garland (2009) - Sparse matrix algorithms for GPUs
- Williams et al. (2007) - SpMV optimization on multicore
- Karypis & Kumar (1998) - METIS graph partitioning

---

**Last Updated:** February 5, 2026  
**Version:** 1.0.0  
**Status:** Production Ready

---

## Appendix A: File Size Reference

```
Source files:         ~30 KB total
Header files:         ~10 KB total
Matrix files:         ~90 MB total (3 matrices)
Executables:          ~200 KB each
Results (full run):   ~2-5 MB total
Logs:                 ~50-100 MB total
```

## Appendix B: Quick Reference Commands

```bash
# Compilation
cd scripts
mpicc -O3 -Wall -lm -I../include -o ../results/spmv_mpi.out \
    ../src/main.c ../src/io_setup.c ../src/computation.c \
    ../src/communication.c ../src/matrix_io.c ../src/mmio.c

# Single run
mpirun -np 4 ../results/spmv_mpi.out ../data/torso1.mtx 10

# Full benchmark
./test.sh

# Cluster submission
qsub benchmark.pbs

# Results extraction
python3 extract_data.py

# Monitor job
qstat -u $USER
tail -f ../results/job_output.out
```

