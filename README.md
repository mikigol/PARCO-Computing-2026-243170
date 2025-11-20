# PARCO Computing 2026-243170: Sparse Matrix-Vector Multiplication Benchmark with OpenMP

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

This project performs a comprehensive benchmark of sparse matrix-vector multiplication (SpMV) operations with focus on **OpenMP parallelization strategies**. It evaluates different scheduling strategies (static, dynamic, guided), chunk sizes (1, 10, 100, 1000), and thread counts (1, 2, 4, 8, 16, 32) across multiple sparse matrices from the SuiteSparse collection.

**Key Outputs:**
- Execution time measurements (90th percentile over 10 iterations)
- Cache performance metrics (L1 and LLC miss rates)
- Automatic visualization and analysis scripts (4 comprehensive graphs + statistics)

**Tested Matrices:**
- **bcsstk14** (1,806×1,806, 32,630 nnz) - Small
- **FEM_3D_thermal2** (147,900×147,900, 3,489,300 nnz) - Medium
- **pdb1HYS** (36,417×36,417, 2,190,591 nnz) - Medium
- **rajat24** (358,172×358,172, 1,948,235 nnz) - Large
- **torso1** (116,158×116,158, 8,516,500 nnz) - Large
---
- THE COMPILATION AND THE RUNNING MUST BE DONE INSIDE THE DIRECTORY Scripts, FOR ALL THE CONFIGURATIONS BOTH LOCAL AND CLUSTER 

## Quick Start

### For Local Testing (Single Matrix)


```bash
# Compile the project
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ./matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm

# Run single sequential execution
./matvec ../Matrix/torso1.mtx 1 none none

# Run single parallel execution (8 threads, static schedule, chunk=100)
./matvec ../Matrix/torso1.mtx 8 static 100
```

### For Full Benchmark on Local Machine

```bash
cd Scripts
chmod +x run.sh
./run.sh
```

**Estimated runtime:** 2-4 hours (depending on CPU)

### For Cluster Submission (UNITN HPC)

```bash
cd Scripts
qsub benchmark.pbs
```

**Estimated runtime:** 6 hours (32 CPU cores, single node)

### Data Analysis

```bash
cd Scripts
python3 lettura_dati.py
```

Results and plots will be generated in `Results/Plots/`

---
## Expected Results

Based on UNITN HPC cluster execution (Intel Xeon Gold 6252N @ 2.3 GHz, 96 logical CPUs):

| Matrix | Best Schedule | Best Chunk | Best Threads | Time (s) | Speedup |
|--------|---------------|-----------|--------------|----------|---------|
| bcsstk14 | guided | 1 | 2 | 0.000062| 1.10× |
| FEM_3D_thermal2 | static | 1000 | 16 | 0.000971 | 4.79× |
| pdb1HYS | static | 1000 | 16 | 0.001094 | 5.33× |
| rajat24 | dynamic | 1000 | 16 | 0.000868 | 3.47× |
| torso1 | static | 100 | 16 | 0.002169 | 5.98× |

**System:** Intel Xeon Gold 6252N @ 2.3 GHz (96 logical CPUs, 4 sockets, L3 cache: 36 MB per socket)

**Average Speedup:** 4.2× (across all matrices and configurations)

---

## System Requirements

### Compiler & Toolchain

| Component | Requirement | Tested Version |
|-----------|-------------|-----------------|
| Compiler | GCC with OpenMP support | . |
| C Standard | C99 or later | ISO/IEC 9899:1999 |
| Build Tool | Make or manual compilation | GNU Make 3.82+ |
| OpenMP | Version 2.0+ | Built-in with GCC |
| Profiler | Linux perf (optional, cluster only) | perf 3.10.0+ |

### Hardware

**Minimum (Local):**
- CPU: Dual-core with OpenMP support
- RAM: 4 GB
- Storage: 500 MB (with matrices)

**Recommended (Cluster):**
- CPU: Intel Xeon Gold 6252N or equivalent (24+ cores per socket)
- RAM: 16 GB per node
- NUMA: Multi-socket NUMA systems for optimal performance

**Tested Hardware (UNITN HPC):**
```
Architecture:  x86_64
CPU: Intel(R) Xeon(R) Gold 6252N @ 2.30GHz
Cores per socket: 24
Sockets: 4 (96 total logical CPUs)
L1 cache: 32K (per core)
L2 cache: 1024K (per core)
L3 cache: 36608K (shared per socket)
Memory: 1TB total (4 NUMA nodes)
Kernel: Linux 3.10.0-1160.53.1.el7.x86_64
```

---

## Installation & Setup

### 1. Clone Repository

```bash
git clone <repository-url>
cd PARCO-Computing-2026-243170
```

### 2. Create Directory Structure

The project structure should be:

```
PARCO-Computing-2026-243170/
├── Header/               # C header files
│   ├── matrix.h
│   ├── csr.h
│   ├── mmio.h
│   └── my_timer.h
├── Src/                  # C source files
│   ├── main.c
│   ├── matrix_io.c
│   ├── csr.c
│   └── mmio.c
├── Scripts/              # Execution and analysis scripts
│   ├── run.sh                           # Local benchmark runner
│   ├── benchmark.pbs                    # PBS cluster submission script
│   ├── lettura_dati.py                  # Data visualization helper
|
├── Matrix/               # Sparse matrix files (.mtx) - NOT in git│ 
│   └── info.txt          # Instructions for matrix setup
├── Results/              # Benchmark output directory
│   ├── Plots/            # Generated visualization plots
│   ├── results_time.csv  # Timing measurements (generated)
│   └── results_perf.csv  # Cache performance metrics (generated)
└── README.md             # This file
```

### 3. Verify Compiler

```bash
gcc --version
# Verify OpenMP support:
gcc -fopenmp -dM -E - < /dev/null | grep -i omp
```

For cluster, load the module:
```bash
module load gcc91
gcc --version  # Should show GCC 9.1+
```

---

## Building the Project

### Local Build

```bash
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ./matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm
```

**Compilation Flags Explanation:**
- `-O3`: Maximum optimization level
- `-Wall`: Enable all warnings
- `-g`: Include debugging symbols (useful for profiling)
- `-fopenmp`: Enable OpenMP parallelization
- `-std=c99`: Use C99 standard
- `-I../Header`: Include path for header files
- `-lm`: Link math library

### Cluster Build (with perf support)

The PBS script automatically compiles two versions:

1. **Time measurement version:**
   ```bash
   gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ./matvec \
       ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm
   ```

2. **Performance profiling version (with PERF_MODE):**
   ```bash
   gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -DPERF_MODE \
       -o ./matvec_perf ../Src/main.c ../Src/matrix_io.c \
       ../Src/csr.c ../Src/mmio.c -lm
   ```

### Troubleshooting Build Issues

If compilation fails:

```bash
# Check for OpenMP headers
find /usr -name omp.h 2>/dev/null

# Verify GCC version meets minimum requirements
gcc -v

# Try alternative compilation (without optimization)
gcc -g -fopenmp -std=c99 -I../Header -o ./matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm
```

---

## Running Benchmarks

### Command-Line Interface

The program accepts the following arguments:

```bash
./matvec <matrix_file> <num_threads> <schedule> <chunk_size>
```

**Parameters:**

| Parameter | Type | Values | Default | Meaning |
|-----------|------|--------|---------|---------|
| `matrix_file` | string | Path to `.mtx` file | - | Sparse matrix in Matrix Market format |
| `num_threads` | int | 1-32 | 1 | Number of OpenMP threads to use |
| `schedule` | string | static, dynamic, guided, none | none | OpenMP scheduling strategy |
| `chunk_size` | int | 1, 10, 100, 1000 | ignored if schedule=none | Chunk size for loop distribution |

**Schedule Types:**
- `none`: Sequential execution (ignores threads and chunk_size parameters)
- `static`: Fixed iteration assignment (best for uniform workloads)
- `dynamic`: Runtime-based distribution (best for irregular workloads)
- `guided`: Hybrid approach (good general-purpose choice)

### Examples

#### Sequential Execution
```bash
./matvec ../Matrix/torso1.mtx 1 none none
```

**Output format:**
```
X.XXXXXX
```
(Time in seconds, 90th percentile over 10 iterations)

#### Parallel Execution - Various Configurations

```bash
# Static schedule, chunk=100, 8 threads
./matvec ../Matrix/torso1.mtx 8 static 100

# Dynamic schedule, chunk=1, 16 threads
./matvec ../Matrix/torso1.mtx 16 dynamic 1

# Guided schedule, chunk=1000, 32 threads
./matvec ../Matrix/torso1.mtx 32 guided 1000
```

### Full Local Benchmark

Run all combinations automatically:

```bash
cd Scripts
chmod +x run.sh
./run.sh
```

**What this does:**
- All 5 matrices
- Sequential (1 thread, none schedule)
- Parallel: 3 schedules × 4 chunk sizes × 6 thread counts = 72 configurations per matrix
- **Total:** 5 matrices × (1 sequential + 72 parallel) = 365 executions
- Generates: `../Results/results_time.csv` with timing data
- Generates: `../Results/results_perf.csv` with cache profiling data
- **Estimated runtime:** 2-4 hours

#### Default Benchmark Parameters (from run.sh)

```bash
MATRICES=("bcsstk14.mtx" "FEM_3D_thermal2.mtx" "pdb1HYS.mtx" "rajat24.mtx" "torso1.mtx")
SCHEDULES=("static" "dynamic" "guided")
CHUNKSIZES=(1 10 100 1000)
THREADS=(1 2 4 8 16 32)
NRUNS=10  # 10 iterations per execution for time measurement
```

**To customize parameters**, edit `Scripts/run.sh` and modify these variables before running.

---

## Project Structure Details

### Source Files

**main.c** - Main entry point
- Command-line argument parsing (matrix file, threads, schedule, chunk)
- Matrix loading and conversion to CSR format
- Timing measurements: 10 iterations, reports 90th percentile
- Optional perf profiling mode (with PERF_MODE flag)

**matrix_io.c / matrix_io.h** - Matrix I/O Operations
- Matrix Market format reading (.mtx files)
- CSR (Compressed Sparse Row) conversion
- Memory allocation and deallocation
- Dimension validation

**csr.c / csr.h** - CSR Matrix Operations
- CSR matrix-vector multiplication kernel (core computation)
- Loop parallelization with OpenMP `#pragma omp parallel for num_threads(num_threads) ` 
- Configurable scheduling and chunk sizes  ex. `#pragma omp parallel for num_threads(num_threads) schedule(static, chunk_size) ` 
- Cache-aware implementation

**mmio.c / mmio.h** - Matrix Market I/O (Reference Implementation)
- Low-level .mtx file parsing and I/O utilities
- Reference implementation from SuiteSparse

### Header Files

- `matrix.h` - Data structures for sparse matrices and CSR format
- `csr.h` - CSR matrix definitions and function prototypes
- `mmio.h` - Matrix Market I/O routines
- `my_timer.h` - High-resolution timing utilities (for precise measurements)

---

## Matrix Management

### Automatic Download Setup

The `Matrix/` directory should contain the `.mtx` files. Due to size constraints (>25 MB per file), matrices are **not included in the Git repository**.

### Downloading Matrices

#### Option 1: Manual Download (Recommended for First Time)

1. Visit [https://sparse.tamu.edu/](https://sparse.tamu.edu/)
2. Search for each matrix name individually
3. Download the `.tar.gz` file
4. Extract and locate the `.mtx` file
5. Copy to `Matrix/` directory

**Step-by-step example for bcsstk14:**

```bash
cd Matrix/

# Download
wget https://sparse.tamu.edu/HB/bcsstk14.tar.gz

# Extract
tar -xzf bcsstk14.tar.gz

# Copy the .mtx file
cp bcsstk14/bcsstk14.mtx .

# Cleanup
rm -rf bcsstk14 bcsstk14.tar.gz

cd ..
```

**Matrix URLs for Reference:**
- bcsstk14: https://sparse.tamu.edu/HB/bcsstk14
- FEM_3D_thermal2: https://sparse.tamu.edu/Botonakis/FEM_3D_thermal2
- pdb1HYS: https://sparse.tamu.edu/Williams/pdb1HYS
- rajat24: https://sparse.tamu.edu/Rajat/rajat24
- torso1: https://sparse.tamu.edu/Norris/torso1

### Verify Matrix Files

```bash
# Check matrices are present
ls -lh Matrix/*.mtx

# Expected output (sizes may vary slightly):
# -rw-r--r--  0.2M  bcsstk14.mtx
# -rw-r--r-- 28.6M  FEM_3D_thermal2.mtx
# -rw-r--r-- 17.9M  pdb1HYS.mtx
# -rw-r--r-- 12.6M  rajat24.mtx
# -rw-r--r-- 70.2M  torso1.mtx
```



## Results Analysis

### Data Files Generated

After running benchmarks, two CSV files are created in `Results/`:

#### 1. `results_time.csv` - Timing Data

**Schema:**
```csv
matrix,mode,schedule,chunk_size,num_threads,90percentile
```

**Fields:**
- `matrix`: Matrix filename (e.g., "torso1.mtx")
- `mode`: "sequential" or "parallel"
- `schedule`: "none" (sequential), "static", "dynamic", "guided"
- `chunk_size`: Chunk size for work distribution (or "none" for sequential)
- `num_threads`: Number of threads used (1, 2, 4, 8, 16, 32)
- `90percentile`: Execution time in seconds (90th percentile of 10 iterations)

**Example rows:**
```csv
torso1.mtx,sequential,none,none,1,0.012970
torso1.mtx,parallel,static,100,8,0.003211
torso1.mtx,parallel,dynamic,1,16,0.004523
torso1.mtx,parallel,guided,1000,32,0.002160
```

#### 2. `results_perf.csv` - Cache Performance Metrics

**Schema:**
```csv
matrix,mode,schedule,chunk_size,num_threads,run_number,l1_loads,l1_misses,l1_miss_rate_percent,llc_loads,llc_misses,llc_miss_rate_percent
```

**Fields:**
- `l1_loads`: Total L1 cache loads
- `l1_misses`: L1 cache load misses
- `l1_miss_rate_percent`: L1 miss rate (%)
- `llc_loads`: Last-level cache (L3) loads
- `llc_misses`: L3 cache load misses
- `llc_miss_rate_percent`: L3 miss rate (%)

**Note:** Cache metrics are based on single iteration per measurement (different from time data which uses 10 iterations, then calculates 90th percentile)

### Python Analysis Script

Run comprehensive analysis and generate visualizations:

```bash
cd Scripts
python3 lettura_dati.py
```

**Prerequisites:**
```bash
pip3 install pandas numpy matplotlib 
```

**Generates 8 PNG files in `Results/Plots/`:**


1. **Strong Scaling Graphs (4 files - one per chunk size)**

- Each file has 1×3 grid: one subplot per schedule type (dynamic, static, guided)
- Shows: how speedup scales with increasing thread count for a specific chunk size
- Ideal scaling line included (black dashed line)
- All 5 matrices plotted per schedule
- Log scale for thread count axis (1, 2, 4, 8, 16, 32)
- Metric: Speedup vs number of threads
- Insight: Which schedule scales best for each matrix at different chunk sizes

2. **GFLOPS Performance Graphs (4 files - one per chunk size)**
- Each file has 1×3 grid: one subplot per schedule type (dynamic, static, guided)
- Compares: sequential (1 thread), 4 threads, 8 threads, 16 threads, 32 threads
- Color-coded bars by thread count (gray for sequential, gold→orange gradient for parallel)
- Values annotated above each bar
- Metric: Computational throughput (GFLOPS)
- Insight: Computational efficiency and memory bandwidth utilization across different chunk sizes

```
GFLOPS = (2 × nnz) / execution_time / 1e9
```
Where `nnz` is the number of non-zero elements in the matrix.

---

## Cluster Execution

### UNITN HPC System Details

**Cluster:** UNITN HPC
**Scheduler:** PBS (Portable Batch System)
**Nodes:** 32 CPU-only nodes (4 Xeon Gold 6252N processors per node)
**Login:** `ssh username@hpc.unitn.it`

### Manual execution
To run your code manually on the UNITN HPC cluster:

### 1. Load Required Modules

After logging in, load the necessary environment modules :
```bash

module load gcc91
module load perf
```
### 2. Create an Interactive Session
Request an interactive session using PBS with the `-I` flag (capital i)
```bash
qsub -I -q queue Name -l select =1: ncpus =64:
mem =1 mb : walltime = 01 : 00 :00
```
- You can adjust the number of CPUs and memory based on your requirements; also you can choose a specific queue.
- The interactive session provides direct access to compute nodes, making it ideal for testing and debugging.
- FOR THE COMPILATION AND EXECUTION IS THE SAME AS FOR THE LOCAL
###  Full Submission

```bash
cd Scripts
qsub benchmark.pbs
```

**Check job status:**
```bash
qstat -u $USER
```

**Check all jobs:**
```bash
qstat
```

**Cancel job:**
```bash
qdel <job_id>
```

### PBS Script Breakdown

The `benchmark.pbs` script performs:

```bash
#!/bin/bash
#PBS -N matvec_benchmark           # Job name
#PBS -o ../Results/benchmark.out   # Stdout file
#PBS -e ../Results/benchmark.err   # Stderr file
#PBS -q short_cpuQ                 # Queue (short = 6 hours max)
#PBS -l walltime=6:00:00           # Walltime limit (6 hours)
#PBS -l select=1:ncpus=32:mem=16gb # 1 node, 32 cores, 16GB RAM
```

**Key execution phases:**

1. **System Information Collection** → `architecture_info.txt`
   - OS, kernel, CPU details
   - Compiler and toolchain versions
   - Module information
   - Cache hierarchy

2. **Compilation** (two versions)
   - Standard version: timing measurements
   - PERF_MODE version: cache profiling with Linux `perf`

3. **Sequential Benchmark** (5 matrices)
   - Single thread, no parallelization
   - Baseline measurements

4. **Parallel Benchmark** (72 configurations × 5 matrices)
   - All schedule/chunk/thread combinations
   - Time and cache profiling

5. **Results Summary**
   - Generates CSV files
   - System information archived

### Output Files

After submission, check results:

```bash
# Monitor progress in real-time
tail -f Results/benchmark.out

# After completion, check for errors
cat Results/benchmark.err

# Verify data files were generated
ls -lh Results/results_*.csv

# Check system info
cat Results/architecture_info.txt

# View generated plots
ls -lh Results/Plots/
```

### Customizing PBS Parameters

Edit `Scripts/benchmark.pbs`:

```bash
# Change job name
#PBS -N my_custom_job_name

# Increase walltime (if needed for larger matrices)
#PBS -l walltime=24:00:00

# Use different queue (long = 72 hours)
#PBS -q long_cpuQ

# Request more memory
#PBS -l select=1:ncpus=32:mem=32gb

# Use multiple nodes (if needed)
#PBS -l select=2:ncpus=32:mem=16gb
```

### Performance Notes for Cluster

1. **Load balancing:** Static scheduling generally recommended for sparse matrices (better cache locality)
2. **NUMA awareness:** L3 cache is shared per socket (24 cores per socket on Xeon Gold)
3. **Memory bandwidth:** Single-node jobs have full access to 1TB memory
4. **Thermal design:** Sustained high-performance execution supported

---

## Troubleshooting

### Compilation Issues



**Error: `-fopenmp` not recognized**
```bash
# Update GCC 
gcc --version

# Install GCC with  OpenMP,  GCC by  default on  macOS don't  support  OpenMP.
brew install gcc

# Verify version installed 
gcc-15 --version  # oppure gcc-14, gcc-13, ecc.

# Compiled with gcc-15
gcc-15 -O3 -std=c99 -fopenmp -I../Header -o ./matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm

```

### Runtime Issues

**Error: `Segmentation fault`**
- Check matrix file exists and is readable: `ls -la Matrix/*.mtx`
- Verify matrix format is valid Matrix Market (.mtx): `head -5 Matrix/bcsstk14.mtx`
- Increase stack size: `ulimit -s unlimited`

### Cluster Issues

**Error: `Job rejected: Walltime exceeds limit`**
- Reduce matrix size or thread count, or
- Request longer walltime: `#PBS -l walltime=24:00:00`

**Error: `Out of memory`**
- Request more memory: `#PBS -l select=1:ncpus=32:mem=32gb`
- Or run on subset of matrices

**Job not starting**
```bash
# Check queue status
qstat -q

# Check node availability
pbsnodes -a

# Submit to different queue
#PBS -q long_cpuQ
```

### Data Analysis Issues

**Python module not found**
```bash
pip3 install pandas numpy matplotlib 
# Or with conda
conda install pandas numpy matplotlib 
```

**CSV file not found**
```bash
# Verify benchmark completed
ls Results/results_*.csv

# If missing, check stderr
cat Results/benchmark.err
```

---

## Common Workflows

### Workflow 1: Quick Local Test (5 minutes)

Perfect for verifying everything works:

```bash
# Compile
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ./matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm

# Test single configuration
./matvec ../Matrix/bcsstk14.mtx 8 static 100

# Expected output: ~0.000150 (seconds)
```

### Workflow 2: Full Benchmark (4 hours)

Complete benchmark of all matrices and configurations:

```bash
# Ensure you're in Scripts directory
cd Scripts

# Make script executable
chmod +x run.sh

# Run full benchmark
./run.sh

# After completion, analyze results
cd ..
python3 Scripts/lettura_dati.py

# View results
ls Results/Plots/
```

### Workflow 3: Cluster Submission (6 hours)

Submit to UNITN HPC for full execution:

```bash
# Navigate to Scripts
cd Scripts

# Submit job
qsub benchmark.pbs

# Get job ID from output, then monitor
qstat -u $USER

# After completion (check every 30 min)
tail -f ../Results/benchmark.out

# Download results
scp -r hpc-user@hpc-head-n1.unitn.it:/path/to/Results ~/local_results/
```

### Workflow 4: Single Matrix Analysis

Focus on one matrix with different configurations:

```bash
# Compile
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o ./matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm

# Test different schedules with 16 threads
echo "Sequential:"
./matvec ../Matrix/torso1.mtx 1 none none

echo "Static:"
./matvec ../Matrix/torso1.mtx 16 static 100

echo "Dynamic:"
./matvec ../Matrix/torso1.mtx 16 dynamic 100

echo "Guided:"
./matvec ../Matrix/torso1.mtx 16 guided 100
```

---

## References

**Resources & Documentation:**
- [SuiteSparse Matrix Collection](https://sparse.tamu.edu/)
- [OpenMP Official Documentation](https://www.openmp.org/)
- [Matrix Market Format Specification](https://math.nist.gov/MatrixMarket/)
- [Linux perf Documentation](https://perf.wiki.kernel.org/)

---

## Report Format

When sharing results, include:

```markdown
## Benchmark Results - PARCO Computing 2026

**System Information:**
- OS: Linux 3.10.0 x86_64
- CPU: Intel Xeon Gold 6252N @ 2.3 GHz (96 logical CPUs, 4 sockets)
- Compiler: GCC 9.1.0 with OpenMP
- Compilation Flags: -O3 -Wall -fopenmp -std=c99

**Benchmark Configuration:**
- Matrices: 5 SuiteSparse matrices (32KB - 70MB)
- Threads: 1, 2, 4, 8, 16, 32
- Schedules: static, dynamic, guided
- Chunk sizes: 1, 10, 100, 1000
- Total configurations: 365 (1 sequential + 72 parallel per matrix)

**Key Results:**
- Best speedup: 5.98× (torso1, 32 threads, guided schedule, chunk=1000)
- Average speedup: 4.2×
- Time measurement: 90th percentile of 10 iterations
- Cache metric: L1 and LLC miss rates from `perf` tool

**Execution Date:** [DATE]
**Duration:** [TIME]
```

---

## Contact & References

**Project:** PARCO Computing 2026-243170
**Institution:** University of Trento
**Course:** Parallel Computing

For questions or issues:
1. Check the troubleshooting section above
2. Review system `Results/architecture_info.txt` for environment details
3. Check `Results/benchmark.err` for error messages
4. Verify matrices are downloaded and readable

---

## License & Citation

If using this benchmark in research, please cite:

```bibtex
@software{parco2026,
  title={Sparse Matrix-Vector Multiplication Benchmark with OpenMP},
  author={Mikele Golemi},
  year={2025},
  institution={University of Trento},
  url={https://github.com/mikigol/PARCO-Computing-2026-243170}
}
```

---

**Last Updated:** November 19, 2025
**Version:** 1.1.0
**Status:** Production Ready
