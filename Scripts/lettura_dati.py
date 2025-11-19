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

# ============================================================================
# PARTE 1: CONFRONTO SCHEDULE CON CHUNK SIZE OTTIMALE (MULTI THREAD)
# ============================================================================

print("=" * 80)
print("PARTE 1: ANALISI CONFRONTO SCHEDULE (MULTI THREAD)")
print("=" * 80)

# Filtro solo i dati paralleli con i 3 schedule di interesse
df_parallel = df_time[df_time['mode'] == 'parallel'].copy()
schedules = ['dynamic', 'static', 'guided']
df_filtered = df_parallel[df_parallel['schedule'].isin(schedules)].copy()

# Ottengo tutte le matrici
matrices = df_filtered['matrix'].unique()

# Thread counts da analizzare
thread_counts_to_plot = [4, 8, 16, 32]

print("\nANALISI: Confronto schedule con miglior chunk_size per vari thread counts")

# Per ogni matrice, trovo il miglior chunk_size globale
best_chunks_per_matrix = {}

for matrix in matrices:
    df_matrix = df_filtered[df_filtered['matrix'] == matrix]
    best_idx = df_matrix['90percentile'].idxmin()
    best_row = df_matrix.loc[best_idx]
    best_chunks_per_matrix[matrix] = best_row['chunk_size']

    print(f"\n{matrix}: best chunk_size = {best_row['chunk_size']}")

# Raccolgo dati per ogni thread count
comparison_results_multi = []

for matrix in matrices:
    best_chunk = best_chunks_per_matrix[matrix]
    df_matrix = df_filtered[df_filtered['matrix'] == matrix]

    for num_threads in thread_counts_to_plot:
        for schedule in schedules:
            df_config = df_matrix[
                (df_matrix['schedule'] == schedule) &
                (df_matrix['chunk_size'] == best_chunk) &
                (df_matrix['num_threads'] == num_threads)
            ]

            if len(df_config) > 0:
                time_ms = df_config.iloc[0]['90percentile'] * 1000
                comparison_results_multi.append({
                    'matrix': matrix,
                    'schedule': schedule,
                    'chunk_size': best_chunk,
                    'num_threads': num_threads,
                    '90percentile_ms': time_ms
                })

comparison_multi_df = pd.DataFrame(comparison_results_multi)

# ============================================================================
# GRAFICO 1: CONFRONTO SCHEDULE CON SUBPLOT PER OGNI THREAD COUNT
# ============================================================================

print("\n" + "=" * 80)
print("CREAZIONE GRAFICO 1: CONFRONTO SCHEDULE (MULTI THREAD)")
print("=" * 80)

# Creo subplot: 2 righe x 2 colonne per i 4 thread counts
fig1, axes = plt.subplots(2, 2, figsize=(18, 12))
axes = axes.flatten()

colors = {'dynamic': '#FF6B6B', 'static': '#4ECDC4', 'guided': '#45B7D1'}
matrices_list = sorted(matrices)
x = np.arange(len(matrices_list))
width = 0.25

for idx, num_threads in enumerate(thread_counts_to_plot):
    ax = axes[idx]

    # Filtro dati per questo numero di thread
    df_threads = comparison_multi_df[comparison_multi_df['num_threads'] == num_threads]

    bars_dict = {}
    for i, schedule in enumerate(schedules):
        schedule_data = df_threads[df_threads['schedule'] == schedule]
        values = []
        for matrix in matrices_list:
            matrix_data = schedule_data[schedule_data['matrix'] == matrix]
            if len(matrix_data) > 0:
                values.append(matrix_data.iloc[0]['90percentile_ms'])
            else:
                values.append(0)

        bars = ax.bar(x + i*width, values, width, label=schedule.capitalize(), 
                      color=colors[schedule], alpha=0.8, edgecolor='black', linewidth=0.7)
        bars_dict[schedule] = (bars, values)

    # Configurazione subplot
    ax.set_xlabel('Matrix', fontsize=11, fontweight='bold')
    ax.set_ylabel('90th Percentile (ms)', fontsize=11, fontweight='bold')
    ax.set_title(f'Threads = {num_threads}', fontsize=13, fontweight='bold', pad=10)

    # Etichette matrici con chunk_size
    matrix_labels = []
    for matrix in matrices_list:
        matrix_name = matrix.replace('.mtx', '')
        chunk = best_chunks_per_matrix[matrix]
        # Nome più corto per evitare sovrapposizioni
        if len(matrix_name) > 12:
            matrix_name = matrix_name[:10] + '..'
        label = f"{matrix_name}\n(chunk={chunk})"
        matrix_labels.append(label)

    ax.set_xticks(x + width)
    ax.set_xticklabels(matrix_labels, rotation=0, ha='center', fontsize=8)
    ax.legend(title='Schedule', fontsize=9, title_fontsize=10, loc='upper left')
    ax.grid(axis='y', alpha=0.3, linestyle='--')

    # Trova il valore massimo per calcolare il limite Y
    all_values = [v for vals in bars_dict.values() for v in vals[1] if v > 0]
    max_val = max(all_values) if all_values else 1

    # Annotazioni valori sopra TUTTE le barre
    for schedule, (bars, values) in bars_dict.items():
        for bar, value in zip(bars, values):
            if value > 0:
                height = bar.get_height()
                ax.text(bar.get_x() + bar.get_width()/2., height,
                       f'{value:.1f}',
                       ha='center', va='bottom', fontsize=7, rotation=0)

    # Imposto limite Y con spazio extra per i numeri sopra le barre
    ax.set_ylim(bottom=0, top=max_val * 1.15)

# Titolo generale
fig1.suptitle('Schedule Comparison with Best Chunk Size - Different Thread Counts', 
              fontsize=16, fontweight='bold', y=0.995)

plt.tight_layout()
plot1_path = os.path.join(PLOTS_DIR, 'confronto_schedule_multithread.png')
plt.savefig(plot1_path, dpi=300, bbox_inches='tight')
print(f"Grafico 1 salvato in: {plot1_path}")

# ============================================================================
# PARTE 2: CALCOLO SPEEDUP
# ============================================================================

print("\n" + "=" * 80)
print("PARTE 2: CALCOLO SPEEDUP")
print("=" * 80)

df_sequential = df_time[df_time['mode'] == 'sequential'].copy()
df_parallel_filtered = df_parallel[df_parallel['schedule'].isin(schedules)].copy()

speedup_results = []

for matrix in matrices:
    print(f"\n{'=' * 80}")
    print(f"MATRICE: {matrix}")
    print(f"{'=' * 80}")

    # Tempo sequenziale
    df_seq_matrix = df_sequential[df_sequential['matrix'] == matrix]
    if len(df_seq_matrix) > 0:
        seq_time = df_seq_matrix.iloc[0]['90percentile']
        seq_time_ms = seq_time * 1000
        print(f"\nTempo Sequential: {seq_time_ms:.3f} ms")
    else:
        print(f"\nATTENZIONE: Nessun dato sequenziale trovato per {matrix}")
        continue

    # Miglior configurazione parallela
    df_par_matrix = df_parallel_filtered[df_parallel_filtered['matrix'] == matrix]
    best_idx = df_par_matrix['90percentile'].idxmin()
    best_row = df_par_matrix.loc[best_idx]

    par_time = best_row['90percentile']
    par_time_ms = par_time * 1000
    speedup = seq_time / par_time

    print(f"\nMiglior configurazione parallela:")
    print(f"  Schedule: {best_row['schedule']}")
    print(f"  Chunk Size: {best_row['chunk_size']}")
    print(f"  Num Threads: {best_row['num_threads']}")
    print(f"  Tempo Parallelo: {par_time_ms:.3f} ms")
    print(f"\n  SPEEDUP: {speedup:.2f}x")

    speedup_results.append({
        'matrix': matrix,
        'sequential_time_ms': seq_time_ms,
        'best_schedule': best_row['schedule'],
        'chunk_size': best_row['chunk_size'],
        'num_threads': best_row['num_threads'],
        'parallel_time_ms': par_time_ms,
        'speedup': speedup
    })

speedup_df = pd.DataFrame(speedup_results)

print("\n" + "=" * 80)
print("STATISTICHE SPEEDUP")
print("=" * 80)
print(f"Speedup medio: {speedup_df['speedup'].mean():.2f}x")
print(f"Speedup massimo: {speedup_df['speedup'].max():.2f}x ({speedup_df.loc[speedup_df['speedup'].idxmax(), 'matrix']})")
print(f"Speedup minimo: {speedup_df['speedup'].min():.2f}x ({speedup_df.loc[speedup_df['speedup'].idxmin(), 'matrix']})")

# ============================================================================
# GRAFICO 2: SPEEDUP
# ============================================================================

print("\n" + "=" * 80)
print("CREAZIONE GRAFICO 2: SPEEDUP")
print("=" * 80)

matrices_list_speedup = speedup_df['matrix'].values
speedups = speedup_df['speedup'].values
x_speedup = np.arange(len(matrices_list_speedup))

fig2, ax2 = plt.subplots(figsize=(14, 8))

colors_speedup = plt.cm.Greens(np.linspace(0.4, 0.9, len(speedups)))
bars = ax2.bar(x_speedup, speedups, color=colors_speedup, edgecolor='black', linewidth=1.2, alpha=0.85)

ax2.axhline(y=1, color='red', linestyle='--', linewidth=2, label='Baseline (no speedup)', alpha=0.7)

for i, (bar, speedup) in enumerate(zip(bars, speedups)):
    height = bar.get_height()
    ax2.text(bar.get_x() + bar.get_width()/2., height,
            f'{speedup:.2f}x',
            ha='center', va='bottom', fontsize=11, fontweight='bold')

matrix_labels_speedup = []
for idx, row in speedup_df.iterrows():
    matrix_name = row['matrix'].replace('.mtx', '')
    label = f"{matrix_name}\n({row['best_schedule']}, chunk={row['chunk_size']}, threads={row['num_threads']})"
    matrix_labels_speedup.append(label)

ax2.set_xlabel('Matrix', fontsize=13, fontweight='bold')
ax2.set_ylabel('Speedup (x)', fontsize=13, fontweight='bold')
ax2.set_title('Speedup: Best Parallel Configuration vs Sequential', 
             fontsize=15, fontweight='bold', pad=20)
ax2.set_xticks(x_speedup)
ax2.set_xticklabels(matrix_labels_speedup, rotation=0, ha='center', fontsize=10)
ax2.legend(fontsize=11)
ax2.grid(axis='y', alpha=0.3, linestyle='--')
ax2.set_ylim(bottom=0, top=max(speedups) * 1.15)

plt.tight_layout()
plot2_path = os.path.join(PLOTS_DIR, 'speedup_grafico.png')
plt.savefig(plot2_path, dpi=300, bbox_inches='tight')
print(f"Grafico 2 salvato in: {plot2_path}")

# ============================================================================
# PARTE 3: SCALABILITÀ (STRONG SCALING) - 3 SUBPLOT PER SCHEDULE
# ============================================================================

print("\n" + "=" * 80)
print("PARTE 3: ANALISI SCALABILITÀ (PER SCHEDULE)")
print("=" * 80)

# Per ogni matrice e schedule, trovo il miglior chunk_size
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
        print("\nATTENZIONE: Nessun dato sequenziale")
        continue

    # Per ogni schedule
    for schedule in schedules:
        df_sched = df_matrix[df_matrix['schedule'] == schedule]

        if len(df_sched) > 0:
            # Trovo il miglior chunk_size per questo schedule
            best_idx_sched = df_sched['90percentile'].idxmin()
            best_chunk_sched = df_sched.loc[best_idx_sched, 'chunk_size']

            print(f"\n{schedule.upper()}: best chunk_size = {best_chunk_sched}")

            thread_counts = sorted(df_sched['num_threads'].unique())

            for num_t in thread_counts:
                df_config = df_sched[
                    (df_sched['chunk_size'] == best_chunk_sched) &
                    (df_sched['num_threads'] == num_t)
                ]

                if len(df_config) > 0:
                    time_val = df_config.iloc[0]['90percentile']
                    time_ms = time_val * 1000
                    speedup_val = seq_time / time_val

                    scalability_data_all.append({
                        'matrix': matrix,
                        'schedule': schedule,
                        'chunk_size': best_chunk_sched,
                        'num_threads': num_t,
                        'time_ms': time_ms,
                        'speedup': speedup_val
                    })

scalability_all_df = pd.DataFrame(scalability_data_all)

# ============================================================================
# GRAFICO 3: SCALABILITÀ CON 3 SUBPLOT (UNO PER SCHEDULE)
# ============================================================================

print("\n" + "=" * 80)
print("CREAZIONE GRAFICO 3: SCALABILITÀ (3 SUBPLOT PER SCHEDULE)")
print("=" * 80)

fig3, axes3 = plt.subplots(1, 3, figsize=(20, 6))

# Colori per matrici
matrix_colors = plt.cm.tab10(np.linspace(0, 1, len(matrices)))

for idx, schedule in enumerate(schedules):
    ax = axes3[idx]

    # Plot per ogni matrice con questo schedule
    for mat_idx, matrix in enumerate(matrices):
        df_comb = scalability_all_df[
            (scalability_all_df['matrix'] == matrix) &
            (scalability_all_df['schedule'] == schedule)
        ]

        if len(df_comb) > 0:
            df_sorted = df_comb.sort_values('num_threads')

            matrix_name = matrix.replace('.mtx', '')
            chunk = best_chunks_per_matrix[matrix]
            label = f"{matrix_name} (chunk={chunk})"

            ax.plot(df_sorted['num_threads'], df_sorted['speedup'], 
                   marker='o', linewidth=2.5, markersize=8, 
                   label=label, 
                   color=matrix_colors[mat_idx],
                   alpha=0.8)

    # Linea ideale
    max_threads = scalability_all_df['num_threads'].max()
    ideal_threads = np.array([1, 2, 4, 8, 16, 32])
    ideal_threads = ideal_threads[ideal_threads <= max_threads]
    ax.plot(ideal_threads, ideal_threads, 'k--', linewidth=2, 
           label='Ideal', alpha=0.5, zorder=0)

    # Configurazione subplot
    ax.set_xlabel('Number of Threads', fontsize=12, fontweight='bold')
    ax.set_ylabel('Speedup (x)', fontsize=12, fontweight='bold')
    ax.set_title(f'{schedule.capitalize()} Schedule', fontsize=14, fontweight='bold', pad=10)
    ax.legend(fontsize=8, loc='upper left')
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.set_xscale('log', base=2)
    ax.set_xticks([1, 2, 4, 8, 16, 32])
    ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())

    # Stesso limite Y per tutti i subplot
    ax.set_ylim(bottom=0, top=7)

# Titolo generale
fig3.suptitle('Strong Scaling per Schedule Type (Best chunk_size per matrix)', 
             fontsize=16, fontweight='bold', y=1.00)

plt.tight_layout()
plot3_path = os.path.join(PLOTS_DIR, 'scalability_per_schedule.png')
plt.savefig(plot3_path, dpi=300, bbox_inches='tight')
print(f"Grafico 3 salvato in: {plot3_path}")

# ============================================================================
# PARTE 4: ANALISI GFLOPS
# ============================================================================

print("\n" + "=" * 80)
print("PARTE 4: CALCOLO GFLOPS")
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
thread_counts_gflops = [8, 16, 32]

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
        print(f"\nSequential: {seq_time:.6f}s → {seq_gflops:.3f} GFLOPS")

        gflops_data.append({
            'matrix': matrix,
            'schedule': 'sequential',
            'num_threads': 1,
            'chunk_size': None,
            'time_s': seq_time,
            'GFLOPS': seq_gflops
        })

    # GFLOPS paralleli per ogni schedule
    df_matrix = df_filtered[df_filtered['matrix'] == matrix]

    for schedule in schedules:
        df_sched = df_matrix[df_matrix['schedule'] == schedule]

        if len(df_sched) > 0:
            # Trovo miglior chunk_size per questo schedule
            best_idx_sched = df_sched['90percentile'].idxmin()
            best_chunk_sched = df_sched.loc[best_idx_sched, 'chunk_size']

            print(f"\n{schedule.upper()}: best chunk_size = {best_chunk_sched}")

            for num_t in thread_counts_gflops:
                df_config = df_sched[
                    (df_sched['chunk_size'] == best_chunk_sched) &
                    (df_sched['num_threads'] == num_t)
                ]

                if len(df_config) > 0:
                    time_val = df_config.iloc[0]['90percentile']
                    gflops_val = (operazioni_totali / time_val) / 1e9

                    print(f"  Threads={num_t}: {time_val:.6f}s → {gflops_val:.3f} GFLOPS")

                    gflops_data.append({
                        'matrix': matrix,
                        'schedule': schedule,
                        'num_threads': num_t,
                        'chunk_size': best_chunk_sched,
                        'time_s': time_val,
                        'GFLOPS': gflops_val
                    })

gflops_df = pd.DataFrame(gflops_data)

# ============================================================================
# GRAFICO 4: GFLOPS CON 3 SUBPLOT (UNO PER SCHEDULE)
# ============================================================================

print("\n" + "=" * 80)
print("CREAZIONE GRAFICO 4: GFLOPS (3 SUBPLOT PER SCHEDULE)")
print("=" * 80)

fig4, axes4 = plt.subplots(1, 3, figsize=(22, 7))

# Colori per thread counts + sequential
thread_colors = {
    1: '#808080',      # Grigio per sequential
    8: '#FFB366',      # Arancione chiaro
    16: '#FF8533',     # Arancione medio
    32: '#FF5500'      # Arancione scuro
}

for idx, schedule in enumerate(schedules):
    ax = axes4[idx]

    # Dati per questo schedule
    df_sched_gflops = gflops_df[gflops_df['schedule'] == schedule]

    # Aggiungo anche i dati sequenziali
    df_seq_gflops = gflops_df[gflops_df['schedule'] == 'sequential']

    # Combino i dati
    df_plot = pd.concat([df_seq_gflops, df_sched_gflops])

    # Matrici ordinate
    matrices_sorted = sorted([m for m in matrices])

    # Preparazione dati per il grafico
    x = np.arange(len(matrices_sorted))
    width = 0.2

    # Plot per ogni thread count
    thread_counts_plot = [1] + thread_counts_gflops  # 1 = sequential

    for i, num_t in enumerate(thread_counts_plot):
        values = []
        for matrix in matrices_sorted:
            df_specific = df_plot[
                (df_plot['matrix'] == matrix) &
                (df_plot['num_threads'] == num_t)
            ]
            if len(df_specific) > 0:
                values.append(df_specific.iloc[0]['GFLOPS'])
            else:
                values.append(0)

        label = 'Sequential' if num_t == 1 else f'{num_t} threads'
        bars = ax.bar(x + i*width, values, width, 
                     label=label,
                     color=thread_colors[num_t], 
                     alpha=0.85, 
                     edgecolor='black', 
                     linewidth=0.8)

        # Annotazioni valori
        for bar, value in zip(bars, values):
            if value > 0:
                height = bar.get_height()
                ax.text(bar.get_x() + bar.get_width()/2., height,
                       f'{value:.2f}',
                       ha='center', va='bottom', fontsize=7, rotation=0)

    # Configurazione subplot
    ax.set_xlabel('Matrix', fontsize=12, fontweight='bold')
    ax.set_ylabel('GFLOPS', fontsize=12, fontweight='bold')
    ax.set_title(f'{schedule.capitalize()} Schedule', 
                fontsize=14, fontweight='bold', pad=10)

    # Etichette matrici CON chunk_size
    matrix_labels_short = []
    for matrix in matrices_sorted:
        matrix_name = matrix.replace('.mtx', '')
        chunk = best_chunks_per_matrix[matrix]
        label = f"{matrix_name}\n(chunk={chunk})"
        matrix_labels_short.append(label)
    ax.set_xticks(x + width * 1.5)
    ax.set_xticklabels(matrix_labels_short, rotation=0, ha='center', fontsize=9)

    ax.legend(fontsize=9, loc='upper left')
    ax.grid(axis='y', alpha=0.3, linestyle='--')

    # Limite Y uniforme
    max_gflops = gflops_df['GFLOPS'].max()
    ax.set_ylim(bottom=0, top=max_gflops * 1.15)

# Titolo generale
fig4.suptitle('GFLOPS Performance: Sequential vs Parallel (8, 16, 32 threads)\nEach schedule uses its optimal chunk_size per matrix', 
             fontsize=16, fontweight='bold', y=1.0)

plt.tight_layout()
plot4_path = os.path.join(PLOTS_DIR, 'gflops_per_schedule.png')
plt.savefig(plot4_path, dpi=300, bbox_inches='tight')
print(f"Grafico 4 salvato in: {plot4_path}")

print("\n" + "=" * 80)
print("STATISTICHE GFLOPS")
print("=" * 80)
print(f"GFLOPS medio sequential: {gflops_df[gflops_df['schedule']=='sequential']['GFLOPS'].mean():.3f}")
print(f"GFLOPS medio parallel (tutti): {gflops_df[gflops_df['schedule']!='sequential']['GFLOPS'].mean():.3f}")
print(f"GFLOPS massimo: {gflops_df['GFLOPS'].max():.3f} ({gflops_df.loc[gflops_df['GFLOPS'].idxmax(), 'matrix']} - {gflops_df.loc[gflops_df['GFLOPS'].idxmax(), 'schedule']} - {gflops_df.loc[gflops_df['GFLOPS'].idxmax(), 'num_threads']} threads)")

plt.show()

print("\n" + "=" * 80)
print("ANALISI COMPLETATA")
print("=" * 80)
print(f"\nTutti i grafici sono stati salvati in: {PLOTS_DIR}")
print("\nFile creati:")
print("  - confronto_schedule_multithread.png (4 subplot per threads 4, 8, 16, 32)")
print("  - speedup_grafico.png")
print("  - scalability_per_schedule.png (3 subplot: dynamic, static, guided)")
print("  - gflops_per_schedule.png (3 subplot: dynamic, static, guided)")
