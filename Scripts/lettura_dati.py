
import pandas as pd
import numpy as np 
import matplotlib.pyplot as plt
import os


TIME_PATH="/Users/mikelegolemi/Desktop/PARCO-Computing-2026-243170/Results/results_time.csv"
PERF_PATH="/Users/mikelegolemi/Desktop/PARCO-Computing-2026-243170/Results/results_perf.csv"
PLOTS_DIR="/Users/mikelegolemi/Desktop/PARCO-Computing-2026-243170/Results/Plots"


# Creo la cartella Plots se non esiste
os.makedirs(PLOTS_DIR, exist_ok=True)
print(f"Cartella per i grafici: {PLOTS_DIR}")


df_time = pd.read_csv(TIME_PATH)
df_perf = pd.read_csv(PERF_PATH)

print("\n" + "="*80)
print("INIZIO ANALISI")
print("="*80)
print(f"\nColonne disponibili: {df_time.columns.tolist()}")
print(f"Chunk sizes disponibili: {sorted(df_time['chunk_size'].unique())}")
print(f"Modalità disponibili: {df_time['mode'].unique()}")
print(f"Schedule disponibili: {df_time['schedule'].unique()}")


# ============================================================================
# PARTE 1: PREPARAZIONE DATI
# ============================================================================


print("\n" + "=" * 80)
print("PARTE 1: PREPARAZIONE DATI")
print("=" * 80)


# Filtro solo i dati paralleli con i 3 schedule di interesse
df_parallel = df_time[df_time['mode'] == 'parallel'].copy()
schedules = ['dynamic', 'static', 'guided']
df_filtered = df_parallel[df_parallel['schedule'].isin(schedules)].copy()

print(f"Dati paralleli disponibili: {len(df_filtered)}")

# Ottengo tutte le matrici
matrices = sorted(df_filtered['matrix'].unique())
print(f"Matrici disponibili: {matrices}")

# Thread counts da analizzare
thread_counts_to_plot = [4, 8, 16, 32]

# Chunk sizes disponibili - PRENDO SOLO QUELLI CHE ESISTONO NEI DATI
available_chunks = sorted(df_filtered['chunk_size'].unique())
print(f"Chunk sizes disponibili nei dati: {available_chunks}")

# Se ci sono almeno 4 chunk sizes, prendo i primi 4. Altrimenti prendo tutti.
if len(available_chunks) >= 4:
    chunk_sizes_to_compare = available_chunks[:4]
else:
    chunk_sizes_to_compare = available_chunks

print(f"Chunk sizes da visualizzare: {chunk_sizes_to_compare}")

df_sequential = df_time[df_time['mode'] == 'sequential'].copy()


# ============================================================================
# PARTE 2: SCALABILITÀ (STRONG SCALING) - MULTIPLE CHUNK SIZE
# ============================================================================


print("\n" + "=" * 80)
print("PARTE 2: ANALISI SCALABILITÀ (STRONG SCALING - MULTIPLE CHUNK SIZE)")
print("=" * 80)


scalability_data_all = []


for matrix in matrices:
    print(f"\n{'=' * 80}")
    print(f"MATRICE: {matrix}")
    print(f"{'=' * 80}")


    df_matrix = df_filtered[df_filtered['matrix'] == matrix]


    # Tempo sequenziale
    df_seq = df_sequential[df_sequential['matrix'] == matrix]
    if len(df_seq) > 0:
        seq_time = df_seq.iloc[0]['90percentile']
    else:
        seq_time = None
        print("ATTENZIONE: Nessun dato sequenziale")
        continue


    # Per ogni schedule
    for schedule in schedules:
        df_sched = df_matrix[df_matrix['schedule'] == schedule]


        if len(df_sched) > 0:
            thread_counts = sorted(df_sched['num_threads'].unique())


            # Per ogni chunk size disponibile
            for chunk_size in sorted(df_sched['chunk_size'].unique()):
                for num_t in thread_counts:
                    df_config = df_sched[
                        (df_sched['chunk_size'] == chunk_size) &
                        (df_sched['num_threads'] == num_t)
                    ]


                    if len(df_config) > 0:
                        time_val = df_config.iloc[0]['90percentile']
                        time_ms = time_val * 1000
                        speedup_val = seq_time / time_val


                        scalability_data_all.append({
                            'matrix': matrix,
                            'schedule': schedule,
                            'chunk_size': chunk_size,
                            'num_threads': num_t,
                            'time_ms': time_ms,
                            'speedup': speedup_val
                        })


scalability_all_df = pd.DataFrame(scalability_data_all)

print(f"\nScalability data totali: {len(scalability_all_df)}")


# ============================================================================
# GRAFICO 1: SCALABILITÀ CON 4 FILE PNG (UNO PER CHUNK SIZE) - 3 SCHEDULE
# ============================================================================


print("\n" + "=" * 80)
print("CREAZIONE GRAFICO 1: STRONG SCALING (MULTIPLE CHUNK SIZE)")
print("=" * 80)


# Per ogni chunk size, creo 3 subplot (uno per schedule)
for chunk_idx, chunk_size in enumerate(chunk_sizes_to_compare):
    fig1, axes1 = plt.subplots(1, 3, figsize=(24, 7))
    
    
    print(f"\n--- Creando grafico per Chunk Size: {chunk_size} ---")
    
    
    # Filtro dati per questo chunk size
    df_chunk_scal = scalability_all_df[scalability_all_df['chunk_size'] == chunk_size]
    print(f"    Righe trovate: {len(df_chunk_scal)}")
    
    
    # Colori per matrici
    matrix_colors = plt.cm.tab10(np.linspace(0, 1, len(matrices)))
    
    
    for sched_idx, schedule in enumerate(schedules):
        ax = axes1[sched_idx]
        
        
        # Plot per ogni matrice con questo schedule
        for mat_idx, matrix in enumerate(matrices):
            df_comb = df_chunk_scal[
                (df_chunk_scal['matrix'] == matrix) &
                (df_chunk_scal['schedule'] == schedule)
            ]
            
            
            if len(df_comb) > 0:
                df_sorted = df_comb.sort_values('num_threads')
                
                
                matrix_name = matrix.replace('.mtx', '')
                label = matrix_name
                
                
                ax.plot(df_sorted['num_threads'], df_sorted['speedup'],
                       marker='o', linewidth=2.5, markersize=8,
                       label=label,
                       color=matrix_colors[mat_idx],
                       alpha=0.8)
        
        
        # Linea ideale
        max_threads = df_chunk_scal['num_threads'].max() if len(df_chunk_scal) > 0 else 32
        ideal_threads = np.array([1, 2, 4, 8, 16, 32])
        ideal_threads = ideal_threads[ideal_threads <= max_threads]
        ax.plot(ideal_threads, ideal_threads, 'k--', linewidth=2,
               label='Ideal', alpha=0.5, zorder=0)
        
        
        # Configurazione subplot
        ax.set_xlabel('Number of Threads', fontsize=12, fontweight='bold')
        ax.set_ylabel('Speedup (x)', fontsize=12, fontweight='bold')
        ax.set_title(f'{schedule.capitalize()} Schedule', fontsize=14, fontweight='bold', pad=10)
        ax.legend(fontsize=8, loc='upper left', ncol=2)
        ax.grid(True, alpha=0.3, linestyle='--')
        ax.set_xscale('log', base=2)
        ax.set_xticks([1, 2, 4, 8, 16, 32])
        ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())
        
        
        # Stesso limite Y per tutti i subplot
        ax.set_ylim(bottom=0, top=7)
    
    
    # Titolo generale
    fig1.suptitle(f'Strong Scaling per Schedule Type - Chunk Size: {chunk_size}',
                 fontsize=16, fontweight='bold', y=1.00)
    
    
    plt.tight_layout()
    plot1_path = os.path.join(PLOTS_DIR, f'01_scalability_chunk_{chunk_idx}.png')
    plt.savefig(plot1_path, dpi=300, bbox_inches='tight')
    print(f"Grafico 1.{chunk_idx} salvato in: {plot1_path}")
    plt.close()


# ============================================================================
# PARTE 3: ANALISI GFLOPS - MULTIPLE CHUNK SIZE
# ============================================================================


print("\n" + "=" * 80)
print("PARTE 3: CALCOLO GFLOP/s (MULTIPLE CHUNK SIZE)")
print("=" * 80)


# Dizionario con nnz per ogni matrice
matrix_nnz = {
    'bcsstk14.mtx': 32630,
    'FEM_3D_thermal2.mtx': 3489300,
    'pdb1HYS.mtx': 2190591,
    'rajat24.mtx': 1948235,
    'torso1.mtx': 8516500
}


# Thread counts da analizzare per GFLOPS
thread_counts_gflops = [4, 8, 16, 32]


# Raccolgo dati GFLOPS
gflops_data = []


for matrix in matrices:
    print(f"\n{'=' * 80}")
    print(f"MATRICE: {matrix}")
    print(f"{'=' * 80}")


    # Ottengo nnz
    nnz = matrix_nnz.get(matrix, 0)
    if nnz == 0:
        print(f"ATTENZIONE: nnz non trovato per {matrix}")
        continue


    operazioni_totali = 2 * nnz
    print(f"nnz: {nnz:,}")
    print(f"Operazioni totali: {operazioni_totali:,}")


    # GFLOPS sequenziale
    df_seq = df_sequential[df_sequential['matrix'] == matrix]
    if len(df_seq) > 0:
        seq_time = df_seq.iloc[0]['90percentile']
        seq_gflops = (operazioni_totali / seq_time) / 1e9
        print(f"Sequential: {seq_time:.6f}s → {seq_gflops:.3f} GFLOP/s")


        gflops_data.append({
            'matrix': matrix,
            'schedule': 'sequential',
            'num_threads': 1,
            'chunk_size': None,
            'time_s': seq_time,
            'GFLOP/s': seq_gflops
        })


    # GFLOPS paralleli per ogni schedule e chunk size
    df_matrix = df_filtered[df_filtered['matrix'] == matrix]


    for schedule in schedules:
        df_sched = df_matrix[df_matrix['schedule'] == schedule]


        if len(df_sched) > 0:
            # Per ogni chunk size disponibile
            for chunk_size in sorted(df_sched['chunk_size'].unique()):
                for num_t in thread_counts_gflops:
                    df_config = df_sched[
                        (df_sched['chunk_size'] == chunk_size) &
                        (df_sched['num_threads'] == num_t)
                    ]


                    if len(df_config) > 0:
                        time_val = df_config.iloc[0]['90percentile']
                        gflops_val = (operazioni_totali / time_val) / 1e9


                        print(f"  {schedule} - Chunk {chunk_size} - Threads {num_t}: {gflops_val:.3f} GFLOP/s")


                        gflops_data.append({
                            'matrix': matrix,
                            'schedule': schedule,
                            'num_threads': num_t,
                            'chunk_size': chunk_size,
                            'time_s': time_val,
                            'GFLOP/s': gflops_val
                        })


gflops_df = pd.DataFrame(gflops_data)

print(f"\nGFLOPS data totali: {len(gflops_df)}")


# ============================================================================
# GRAFICO 2: GFLOPS CON 4 FILE PNG (UNO PER CHUNK SIZE) - 3 SCHEDULE
# ============================================================================


print("\n" + "=" * 80)
print("CREAZIONE GRAFICO 2: GFLOP/s (MULTIPLE CHUNK SIZE)")
print("=" * 80)


# Per ogni chunk size, creo 3 subplot (uno per schedule)
for chunk_idx, chunk_size in enumerate(chunk_sizes_to_compare):
    fig2, axes2 = plt.subplots(1, 3, figsize=(26, 8))
    
    
    print(f"\n--- Creando grafico GFLOPS per Chunk Size: {chunk_size} ---")
    
    
    # Filtro dati per questo chunk size
    df_chunk_gflops = gflops_df[(gflops_df['chunk_size'] == chunk_size) | (gflops_df['schedule'] == 'sequential')]
    print(f"    Righe trovate: {len(df_chunk_gflops)}")
    
    
    # Colori per thread counts
    thread_colors = {
        1: '#808080',      # Grigio per sequential
        4: '#FFD700',      # Oro
        8: '#FFB366',      # Arancione chiaro
        16: '#FF8533',     # Arancione medio
        32: '#FF5500'      # Arancione scuro
    }
    
    
    matrices_sorted = matrices
    x_gflops = np.arange(len(matrices_sorted))
    width_gflops = 0.15
    
    
    for sched_idx, schedule in enumerate(schedules):
        ax = axes2[sched_idx]
        
        
        # Dati per questo schedule + sequential
        if schedule == 'sequential':
            df_sched_gflops = gflops_df[gflops_df['schedule'] == 'sequential']
        else:
            df_sched_gflops_temp = df_chunk_gflops[df_chunk_gflops['schedule'] == schedule]
            df_seq_gflops = gflops_df[gflops_df['schedule'] == 'sequential']
            df_sched_gflops = pd.concat([df_seq_gflops, df_sched_gflops_temp])
        
        
        # Preparazione dati per il grafico
        thread_counts_plot = [1] + thread_counts_gflops
        
        
        # Plot per ogni thread count
        for thread_idx, num_t in enumerate(thread_counts_plot):
            values = []
            for matrix in matrices_sorted:
                df_specific = df_sched_gflops[
                    (df_sched_gflops['matrix'] == matrix) &
                    (df_sched_gflops['num_threads'] == num_t)
                ]
                if len(df_specific) > 0:
                    values.append(df_specific.iloc[0]['GFLOP/s'])
                else:
                    values.append(0)
            
            
            label = 'Sequential' if num_t == 1 else f'{num_t} threads'
            bars = ax.bar(x_gflops + thread_idx*width_gflops, values, width_gflops,
                         label=label,
                         color=thread_colors[num_t],
                         alpha=0.85,
                         edgecolor='black',
                         linewidth=0.8)
            
            
            # Annotazioni
            for bar, value in zip(bars, values):
                if value > 0:
                    height = bar.get_height()
                    ax.text(bar.get_x() + bar.get_width()/2., height,
                           f'{value:.1f}',
                           ha='center', va='bottom', fontsize=6, rotation=0)
        
        
        # Configurazione subplot
        ax.set_xlabel('Matrix', fontsize=12, fontweight='bold')
        ax.set_ylabel('GFLOP/s', fontsize=12, fontweight='bold')
        ax.set_title(f'{schedule.capitalize()} Schedule', fontsize=14, fontweight='bold', pad=10)
        
        
        matrix_labels_short = [m.replace('.mtx', '') for m in matrices_sorted]
        ax.set_xticks(x_gflops + width_gflops * 2)
        ax.set_xticklabels(matrix_labels_short, rotation=45, ha='right', fontsize=10)
        
        
        ax.legend(fontsize=9, loc='upper left', ncol=2)
        ax.grid(axis='y', alpha=0.3, linestyle='--')
        
        
        # Limite Y
        max_gflops = gflops_df['GFLOP/s'].max() if len(gflops_df) > 0 else 1
        ax.set_ylim(bottom=0, top=max_gflops * 1.15)
    
    
    # Titolo generale
    fig2.suptitle(f'GFLOP/s Performance per Schedule - Chunk Size: {chunk_size}',
                 fontsize=16, fontweight='bold', y=0.995)
    
    
    plt.tight_layout()
    plot2_path = os.path.join(PLOTS_DIR, f'02_gflops_chunk_{chunk_idx}.png')
    plt.savefig(plot2_path, dpi=300, bbox_inches='tight')
    print(f"Grafico 2.{chunk_idx} salvato in: {plot2_path}")
    plt.close()


if len(gflops_df) > 0:
    print("\n" + "=" * 80)
    print("STATISTICHE GFLOP/s")
    print("=" * 80)
    print(f"GFLOP/s medio sequential: {gflops_df[gflops_df['schedule']=='sequential']['GFLOP/s'].mean():.3f}")
    print(f"GFLOP/s medio parallel (tutti): {gflops_df[gflops_df['schedule']!='sequential']['GFLOP/s'].mean():.3f}")
    print(f"GFLOP/s massimo: {gflops_df['GFLOP/s'].max():.3f}")


print("\n" + "=" * 80)
print("ANALISI COMPLETATA")
print("=" * 80)
print(f"\nTutti i grafici sono stati salvati in: {PLOTS_DIR}")
print("\nFile creati:")
print("  - 01_scalability_chunk_0.png")
print("  - 01_scalability_chunk_1.png")
print("  - 01_scalability_chunk_2.png")
print("  - 01_scalability_chunk_3.png")
print("  - 02_gflops_chunk_0.png")
print("  - 02_gflops_chunk_1.png")
print("  - 02_gflops_chunk_2.png")
print("  - 02_gflops_chunk_3.png")

plt.show()
