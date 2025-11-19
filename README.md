# PARCO Computing 2026-243170: Sparse Matrix-Vector Multiplication Benchmark

## Table of Contents

1. [Project Overview](#project-overview)
2. [Quick Start](#quick-start)
3. [System Requirements](#system-requirements)
4. [Installation & Setup](#installation--setup)
5. [Building the Project](#building-the-project)
6. [Running Benchmarks](#running-benchmarks)
7. [Matrix Management](#matrix-management)
8. [Results Analysis](#results-analysis)
9. [Cluster Execution](#cluster-execution)
10. [Troubleshooting](#troubleshooting)


---

## Project Overview

This project performs a comprehensive benchmark of sparse matrix-vector multiplication (SpMV) operations with focus on **OpenMP parallelization strategies**. It evaluates different scheduling strategies (static, dynamic, guided), chunk sizes (1, 10, 100, 1000), and thread counts (1, 2, 4, 8, 16, 32) across multiple sparse matrices from the SuiteSparse collection.

**Key Outputs:**
- Execution time measurements (90th percentile over 10 iterations)
- Cache performance metrics (L1 and LLC miss rates)
- Automatic visualization and analysis scripts

**Tested Matrices:**
- bcsstk14 (1,806×1,806, 32,630 nnz)
- FEM_3D_thermal2 (147,900×147,900, 3,489,300 nnz)
- pdb1HYS (36,417×36,417, 2,190,591 nnz)
- rajat24 (358,172×358,172, 1,948,235 nnz)
- torso1 (116,158×116,158, 8,516,500 nnz)

---

## Quick Start

### For Local Testing (Single Matrix)

```bash
# Compile the project
cd Scripts
chmod +x run.sh
cd ..

# Build the executable
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o matvec \
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

**Estimated runtime:** 2-4 hours (32 CPU cores, single node)

### Data Analysis

```bash
cd Scripts
python3 lettura_dati.py
```

Results and plots will be generated in `Results/Plots/`

---

## System Requirements

### Compiler & Toolchain

| Component | Requirement | Tested Version |
|-----------|-------------|-----------------|
| Compiler | GCC with OpenMP support | 4.8.5 (cluster), 11.2+ (recommended) |
| C Standard | C99 or later | ISO/IEC 9899:1999 |
| Build Tool | Make or manual compilation | GNU Make 3.82+ |
| OpenMP | Version 2.0+ | Built-in with GCC |
| Profiler | Linux perf (optional) | perf 3.10.0+ |

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
│   └── mmio.h
|   └── my_timer.h
├── Src/                  # C source files
│   ├── main.c
│   ├── matrix_io.c
│   ├── csr.c
│   └── mmio.c
├── Scripts/              # Execution and analysis scripts
│   ├── run.sh                        # Local benchmark runner
│   ├── benchmark.pbs                 # PBS cluster submission script
│   ├── lettura_dati.py              # Data visualization helper
│ 
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
gcc -O3 -Wall -g -fopenmp -std=c99 -I../Header -o matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm
```

**Compilation Flags Explanation:**
- `-O3`: Maximum optimization level
- `-Wall`: Enable all warnings
- `-g`: Include debugging symbols
- `-fopenmp`: Enable OpenMP parallelization
- `-std=c99`: Use C99 standard
- `-I./Header`: Include path for header files
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
gcc -g -fopenmp -std=c99 -I../Header -o matvec \
    ../Src/main.c ../Src/matrix_io.c ../Src/csr.c ../Src/mmio.c -lm
```

---

## Running Benchmarks

### Local Execution

#### Single Matrix - Sequential

```bash
./matvec ../Matrix/torso1.mtx 1 none none
```

**Output format:**
```
X.XXXXXX
```
(Time in seconds, 90th percentile over 10 iterations)

#### Single Matrix - Parallel

```bash
# Static schedule, 100 chunk size, 8 threads
./matvec ../Matrix/torso1.mtx 8 static 100

# Dynamic schedule, 1 chunk size, 16 threads
./matvec ../Matrix/torso1.mtx 16 dynamic 1

# Guided schedule, 1000 chunk size, 32 threads
./matvec ../Matrix/torso1.mtx 32 guided 1000
```

#### Parameters Explanation

| Parameter | Values | Default | Meaning |
|-----------|--------|---------|---------|
| `matrix_file` | Path to `.mtx` file | - | Sparse matrix in Matrix Market format |
| `num_threads` | 1-96 | 1 | Number of OpenMP threads to use |
| `schedule` | static, dynamic, guided, none | none | OpenMP scheduling strategy |
| `chunk_size` | 1, 10, 100, 1000 | ignored if schedule=none | Chunk size for loop distribution |

**Schedule Types:**
- `none`: Sequential execution (ignores threads, chunk_size)
- `static`: Fixed iteration assignment (best for uniform workloads)
- `dynamic`: Runtime-based distribution (best for irregular workloads)
- `guided`: Hybrid approach (good general-purpose choice)

#### Full Local Benchmark

Run all combinations automatically:

```bash
cd Scripts
chmod +x run.sh
./run.sh
```

This executes:
- All 5 matrices
- Sequential (1 thread, none schedule)
- Parallel: 3 schedules × 4 chunk sizes × 6 thread counts = 72 configurations per matrix
- **Total:** 5 matrices × (1 sequential + 72 parallel) = 365 executions
- Generates: `results_time.csv` with timing data
- Estimated runtime: 2-4 hours

#### Default Benchmark Parameters (from run.sh)

```bash
MATRICES=("bcsstk14.mtx" "FEM_3D_thermal2.mtx" "pdb1HYS.mtx" "rajat24.mtx" "torso1.mtx")
SCHEDULES=("static" "dynamic" "guided")
CHUNKSIZES=(1 10 100 1000)
THREADS=(1 2 4 8 16 32)
NRUNS=10  # 10 iterations per execution for time measurement
```

**To customize parameters**, edit `Scripts/run.sh` and modify these variables.

---

## Matrix Management

### Automatic Download Setup

The `Matrix/` directory should contain the `.mtx` files. Due to size constraints (>25 MB), matrices are **not included in the Git repository**.

### Downloading Matrices

#### Option 1: Manual Download

1. Visit https://sparse.tamu.edu/
2. Search for each matrix name
3. Download the `.tar.gz` file
4. Extract: `tar -xzf matrix_name.tar.gz`
5. Locate the `.mtx` file and copy to `Matrix/` directory

**Example:**
```bash
# Download bcsstk14
cd Matrix/
wget https://sparse.tamu.edu/mat/Oberwolfach/bcsstk14.tar.gz
tar -xzf bcsstk14.tar.gz
cp bcsstk14/bcsstk14.mtx .
rm -rf bcsstk14 bcsstk14.tar.gz
cd ..
```

### Verify Matrix Files

```bash
# Check matrices are present
ls -lh Matrix/*.mtx

# Expected output:
# -rw-r--r-- 1 user group  0.2M bcsstk14.mtx
# -rw-r--r-- 1 user group 28.6M FEM_3D_thermal2.mtx
# -rw-r--r-- 1 user group 17.9M pdb1HYS.mtx
# -rw-r--r-- 1 user group 12.6M rajat24.mtx
# -rw-r--r-- 1 user group 70.2M torso1.mtx
```


## Results Analysis

### Data Files Generated

After running benchmarks, two CSV files are created:

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
- `num_threads`: Number of threads used (1-32)
- `90percentile`: Execution time in seconds (90th percentile of 10 iterations)

**Example rows:**
```csv
torso1.mtx,sequential,none,none,1,0.012970
torso1.mtx,parallel,static,100,8,0.003211
torso1.mtx,parallel,dynamic,1,16,0.004523
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

**Note:** Based on single iteration per measurement (different from time data which uses 10 iterations)

### Python Analysis Script

Run comprehensive analysis:

```bash
cd Scripts
python3 analisi_completa_con_gflops.py
```

**Generates 4 PNG files in `Results/Plots/`:**

1. **confronto_schedule_multithread.png**
   - 2×2 grid: threads = 4, 8, 16, 32
   - Compares: dynamic, static, guided for each matrix
   - Shows: chunk size next to each matrix name
   - Metric: 90th percentile execution time (ms)

2. **speedup_grafico.png**
   - Best parallel vs sequential speedup
   - One bar per matrix
   - Metric: Speedup factor (x times)

3. **scalability_per_schedule.png**
   - 1×3 grid: one subplot per schedule type
   - Shows: how speedup scales with thread count
   - Ideal scaling line included
   - Metric: Speedup vs number of threads (log scale)

4. **gflops_per_schedule.png**
   - 1×3 grid: one subplot per schedule type
   - Compares: sequential, 8 threads, 16 threads, 32 threads
   - Shows: chunk size for each matrix
   - Metric: Computational throughput (GFLOPS)

**GFLOPS Calculation:**
```
GFLOPS = (2 × nnz) / execution_time / 1e9
```
Where `nnz` is the number of non-zero elements in the matrix.

### Example Usage - Python

```python
import pandas as pd
import matplotlib.pyplot as plt

# Load timing data
df_time = pd.read_csv('Results/results_time.csv')

# Find best configuration for torso1
torso_data = df_time[df_time['matrix'] == 'torso1.mtx']
best_config = torso_data.loc[torso_data['90percentile'].idxmin()]

print(f"Best config for torso1:")
print(f"  Schedule: {best_config['schedule']}")
print(f"  Threads: {best_config['num_threads']}")
print(f"  Chunk Size: {best_config['chunk_size']}")
print(f"  Time: {best_config['90percentile']:.6f}s")

# Calculate speedup
seq_time = torso_data[torso_data['mode'] == 'sequential'].iloc[0]['90percentile']
speedup = seq_time / best_config['90percentile']
print(f"  Speedup: {speedup:.2f}x")
```

---

## Cluster Execution

### UNITN HPC System Details

**Cluster:** UNITN HPC
**Scheduler:** PBS (Portable Batch System)
**Nodes:** 32 CPU-only nodes (4 Xeon Gold 6252N per node)
**Login:** `hpc-head-n1.unitn.it`

### Submission

```bash
cd Scripts
qsub benchmark.pbs
```

**Check job status:**
```bash
qstat -u $USER
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
#PBS -o ../Results/benchmark.out   # Stdout
#PBS -e ../Results/benchmark.err   # Stderr
#PBS -q short_cpuQ                 # Queue (short = 6 hours max)
#PBS -l walltime=6:00:00           # Walltime limit
#PBS -l select=1:ncpus=32:mem=16gb # 1 node, 32 cores, 16GB RAM
```

**Key sections:**
1. System information collection → `architecture_info.txt`
2. Compilation (two versions: standard + perf)
3. Sequential benchmark (5 matrices)
4. Parallel benchmark (72 configurations × 5 matrices)
5. Cache profiling with Linux `perf` tool

### Output Files

After submission, check results:

```bash
# Monitor progress
tail -f Results/benchmark.out

# After completion
cat Results/benchmark.err          # Check for errors
ls -lh Results/results_*.csv       # Generated data files
ls -lh Results/architecture_info.txt  # System info
```

### Customizing PBS Parameters

Edit `Scripts/benchmark.pbs`:

```bash
# Change job name
#PBS -N my_custom_name

# Increase walltime (if matrices are larger)
#PBS -l walltime=24:00:00

# Use different queue (long = 72 hours)
#PBS -q long_cpuQ

# Request more memory
#PBS -l select=1:ncpus=32:mem=32gb

# Use multiple nodes (if needed)
#PBS -l select=2:ncpus=32:mem=16gb
```

### Performance Notes for Cluster

1. **Load balancing:** Static scheduling recommended for sparse matrices
2. **NUMA awareness:** Consider thread pinning for optimal performance
3. **Cache utilization:** L3 cache is shared per socket (24 cores per socket)
4. **Network:** Single-node jobs don't use network overhead

---

## Troubleshooting

### Compilation Issues

**Error: `fatal error: omp.h: No such file or directory`**
```bash
# Solution: Install OpenMP development package
# Ubuntu/Debian
sudo apt-get install libomp-dev

# CentOS/RHEL
sudo yum install libomp-devel

# macOS
brew install libomp
```

**Error: `-fopenmp` not recognized**
```bash
# Use alternative compiler
clang -fopenmp -O3 -std=c99 -I../Header ...
# OR update GCC
gcc --version  # Must be 4.9 or newer
```

### Runtime Issues

**Error: `Segmentation fault`**
- Check matrix file exists and is readable
- Verify matrix format is valid Matrix Market (.mtx)
- Increase stack size: `ulimit -s unlimited`

**Error: `Too many open files`**
- Increase file descriptor limit: `ulimit -n 4096`

**Error: OpenMP threads not being used**
```bash
# Check thread count
export OMP_NUM_THREADS=8
./matvec ../Matrix/torso1.mtx 8 static 100

# Verify with environment
echo $OMP_NUM_THREADS
```

### Cluster Issues

**Error: `Job rejected: Walltime exceeds limit`**
- Reduce matrix size or thread count
- Or request longer walltime: `#PBS -l walltime=24:00:00`

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
pip install pandas numpy matplotlib seaborn
# Or with conda
conda install pandas numpy matplotlib seaborn
```#

**CSV file not found**
```bash
# Verify benchmark completed
ls Results/results_*.csv
# If missing, check stderr
cat Results/benchmark.err
```


### Report Format

When sharing results, include:

```markdown
## Benchmark Results

**System:**
- OS: Linux 3.10.0 x86_64
- CPU: Intel Xeon Gold 6252N @ 2.3 GHz (96 logical CPUs, 4 sockets)
- Compiler: GCC 9.1.0 with OpenMP
- Flags: -O3 -Wall -fopenmp -std=c99

**Matrices:** 5 SuiteSparse matrices (32KB - 70MB)

**Configurations Tested:**
- Threads: 1, 2, 4, 8, 16, 32
- Schedules: static, dynamic, guided
- Chunk sizes: 1, 10, 100, 1000

**Key Results:**
- Best speedup: 5.98× (torso1, 32 threads)
- Average speedup: 3.45×
- Time measurement: 90th percentile of 10 iterations

**Timestamp:** $(date)
```

---

## Contact & References

**Project:** PARCO Computing 2026-243170
**Course:** Parallel Computing, University of Trento

For questions or issues:
1. Check the troubleshooting section
2. Review system `architecture_info.txt`
3. Check `benchmark.err` for error details
4. Verify matrices are downloaded correctly

---

## License & Citation

If using this benchmark in research, please cite:
```bibtex
@software{parco2026,
  title={Sparse Matrix-Vector Multiplication Benchmark with OpenMP},
  author={},
  year={2025},
  institution={University of Trento},
  url={https://github.com/username/PARCO-Computing-2026-243170}
}
```

---

**Last Updated:** November 2025
**Version:** 1.0.0
