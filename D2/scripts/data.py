import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import numpy as np
import matplotlib.ticker as ticker

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
INPUT_DIR = os.path.join(CURRENT_DIR, '../results/tables')     
RAW_DIR = os.path.join(CURRENT_DIR, '../results')               
OUTPUT_DIR = os.path.join(CURRENT_DIR, '../plots')

if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

sns.set_style("whitegrid")
plt.rcParams.update({
    'font.size': 12,
    'text.usetex': False, 
    'font.family': 'serif'
})



def format_log2_x_axis(x, pos):
    if x <= 0: return ""
    log_val = np.log2(x)
    if np.isclose(log_val, round(log_val)):
        return r'$2^{%d}$' % int(round(log_val))
    return ""

def format_speedup_y(x, pos):
    return f"{int(x)}x" if x.is_integer() else f"{x:.1f}x"

def format_efficiency_y(x, pos):
    return f"{int(x)}%"

def calculate_metrics_dataframe(df, is_weak_scaling):
    df = df.sort_values(by='num_procs').copy()
    if df.empty: return df
    
    base_row = df.iloc[0]
    base_time = base_row['p90_time']
    base_procs = base_row['num_procs']
    
    if is_weak_scaling:
        df['speedup'] = (df['num_procs'] / base_procs) * (base_time / df['p90_time'])
        df['efficiency'] = base_time / df['p90_time']
    else:
        df['speedup'] = (base_time / df['p90_time']) * base_procs
        df['efficiency'] = df['speedup'] / (df['num_procs'] / base_procs)

    return df


def plot_strong_scaling_single(filename):
    if 'synthetic' in filename.lower() or 'weak' in filename.lower(): return

    filepath = os.path.join(INPUT_DIR, filename)
    if not os.path.exists(filepath): return

    df = pd.read_csv(filepath)
    matrix_name = filename.replace('summary_', '').replace('.csv', '').replace('_combined', '')
    
    plot_data = pd.DataFrame()
    for mode in df['mode'].unique():
        sub = calculate_metrics_dataframe(df[df['mode'] == mode], is_weak_scaling=False)
        plot_data = pd.concat([plot_data, sub])

    if plot_data.empty: return

    fig, ax = plt.subplots(figsize=(8, 6))
    min_p, max_p = plot_data['num_procs'].min(), plot_data['num_procs'].max()
    
    ax.plot([min_p, max_p], [min_p, max_p], '--', color='#666666', label='Ideal ($y=x$)', linewidth=1.5)

    sns.lineplot(data=plot_data, x='num_procs', y='speedup', hue='mode', style='mode', 
                 markers=True, dashes=False, linewidth=2.5, markersize=9, ax=ax)

    ax.set_xscale('log', base=2); ax.set_yscale('log', base=2)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(format_log2_x_axis))
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(format_speedup_y))
    ax.yaxis.set_major_locator(ticker.LogLocator(base=2.0, numticks=15))

    ax.set_xlabel("Number of Processes ($P$)", fontsize=12)
    ax.set_ylabel("Speedup ($x$)", fontsize=12)
    ax.set_title(f"Strong Scaling Speedup: {matrix_name}", fontsize=14, fontweight='bold')
    ax.legend(fontsize=11)
    ax.grid(True, which="major", linestyle='--', alpha=0.5)
    
    save_path = os.path.join(OUTPUT_DIR, f"Strong_Speedup_{matrix_name}.png")
    plt.tight_layout()
    plt.savefig(save_path, dpi=300)
    plt.close()
    print(f"[PLOT] Speedup Generato: {save_path}")

def plot_weak_scaling_comparison(filename):
    if not ('synthetic' in filename.lower() or 'weak' in filename.lower()): return

    filepath = os.path.join(INPUT_DIR, filename)
    df = pd.read_csv(filepath)
    matrix_name = "Synthetic Matrix (Weak Scaling)" 

    plot_data = pd.DataFrame()
    for mode in df['mode'].unique():
        sub = calculate_metrics_dataframe(df[df['mode'] == mode], is_weak_scaling=True)
        plot_data = pd.concat([plot_data, sub])

    if plot_data.empty: return

    fig, ax = plt.subplots(figsize=(8, 6))
    min_p, max_p = plot_data['num_procs'].min(), plot_data['num_procs'].max()

    ax.plot([min_p, max_p], [min_p, max_p], '--', color='#444444', label='Ideal Scaled Speedup', linewidth=1.5)

    sns.lineplot(data=plot_data, x='num_procs', y='speedup', hue='mode', style='mode', 
                 markers=True, dashes=False, linewidth=2.5, markersize=10, ax=ax)

    ax.set_xscale('log', base=2); ax.set_yscale('log', base=2)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(format_log2_x_axis))
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(format_speedup_y))
    ax.yaxis.set_major_locator(ticker.LogLocator(base=2.0, numticks=15))

    ax.set_xlabel("Number of Processes ($P$)", fontsize=12)
    ax.set_ylabel("Scaled Speedup ($x$)", fontsize=12)
    ax.set_title(f"Weak Scaling Comparison: MPI vs Hybrid\n{matrix_name}", fontsize=14, fontweight='bold')
    ax.legend(title="Configuration", fontsize=11)
    ax.grid(True, which="major", linestyle='--', alpha=0.5)

    save_path = os.path.join(OUTPUT_DIR, f"Weak_Scaling_Comparison_{filename.replace('.csv','')}.png")
    plt.tight_layout()
    plt.savefig(save_path, dpi=300)
    plt.close()
    print(f"[PLOT] Weak Scaling Generato: {save_path}")

def plot_combined_efficiency_mpi_only():
    print("\n--- Generazione Grafico Efficienza Combinato ---")
    files = [f for f in os.listdir(INPUT_DIR) if f.startswith('summary_') and f.endswith('.csv')]
    combined_data = pd.DataFrame()

    for filename in files:
        filepath = os.path.join(INPUT_DIR, filename)
        df = pd.read_csv(filepath)
        is_weak = 'synthetic' in filename.lower() or 'weak' in filename.lower()
        matrix_name = filename.replace('summary_', '').replace('.csv', '').replace('_combined', '')
        
        df_mpi = df[df['mode'].astype(str).str.contains("Pure MPI", case=False)].copy()
        if df_mpi.empty: continue

        df_mpi = calculate_metrics_dataframe(df_mpi, is_weak)
        df_mpi['efficiency_pct'] = df_mpi['efficiency'] * 100 
        df_mpi['Matrix'] = matrix_name
        combined_data = pd.concat([combined_data, df_mpi])

    if combined_data.empty: return

    fig, ax = plt.subplots(figsize=(9, 7))
    min_p, max_p = combined_data['num_procs'].min(), combined_data['num_procs'].max()
    ax.hlines(y=100, xmin=min_p, xmax=max_p, colors='black', linestyles='--', label='Ideal Efficiency', linewidth=1.5)

    sns.lineplot(data=combined_data, x='num_procs', y='efficiency_pct', hue='Matrix', style='Matrix',
                 markers=True, dashes=False, linewidth=2.5, markersize=9, palette="tab10", ax=ax)

    ax.set_xscale('log', base=2)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(format_log2_x_axis))
    ax.set_ylim(0, 120)
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(format_efficiency_y))

    ax.set_xlabel("Number of Processes ($P$)", fontsize=13)
    ax.set_ylabel("Efficiency (%)", fontsize=13)
    ax.set_title("MPI Efficiency Comparison: All Matrices", fontsize=15, fontweight='bold')
    ax.legend(title="Matrix Name", fontsize=11)
    ax.grid(True, which="major", linestyle='--', alpha=0.5)

    save_path = os.path.join(OUTPUT_DIR, "Combined_Efficiency_PureMPI.png")
    plt.tight_layout()
    plt.savefig(save_path, dpi=300)
    plt.close()
    print(f"[PLOT] Efficiency Combinata Generata: {save_path}")


def plot_mpi_comm_vs_comp_stacked_ms_clean():
    print("\n--- Generazione Grafici Comm vs Comp (Strong - Dynamic Log) ---")
    
    filename = 'strong_scaling_all.csv'
    filepath = os.path.join(RAW_DIR, filename)

    if not os.path.exists(filepath):
        print(f"[WARN] File raw non trovato: {filepath}")
        return

    df = pd.read_csv(filepath, dtype=str)
    cols = ['elapsed_time', 'comm_time', 'num_procs']
    for col in cols:
        df[col] = pd.to_numeric(df[col], errors='coerce')
    
    df = df.dropna(subset=cols)
    df['num_procs'] = df['num_procs'].astype(int)

    valid_procs = [1, 2, 4, 8, 16, 32, 64, 128]
    df = df[df['num_procs'].isin(valid_procs)]

    if df.empty:
        print("[ERR] Nessun dato valido trovato.")
        return

    df['clean_matrix'] = df['matrix_name'].apply(
        lambda x: 'synthetic' if 'synthetic' in str(x) else os.path.basename(str(x)).replace('.mtx', '')
    )

    unique_matrices = df['clean_matrix'].unique()

    for matrix in unique_matrices:
        if 'synthetic' in matrix.lower(): continue

        sub_df = df[df['clean_matrix'] == matrix].copy()
        if sub_df.empty: continue

        grouped = sub_df.groupby('num_procs').agg({
            'elapsed_time': 'mean',
            'comm_time': 'mean'
        }).reset_index().sort_values('num_procs')

        grouped['elapsed_time_ms'] = grouped['elapsed_time'] * 1000
        grouped['comm_time_ms'] = grouped['comm_time'] * 1000
        grouped['comp_time_ms'] = grouped['elapsed_time_ms'] - grouped['comm_time_ms']
        grouped['comp_time_ms'] = grouped['comp_time_ms'].clip(lower=0)

        procs = grouped['num_procs'].astype(str).tolist()
        comp_vals = grouped['comp_time_ms'].tolist()
        comm_vals = grouped['comm_time_ms'].tolist()
        
        fig, ax = plt.subplots(figsize=(10, 6))
        x_pos = np.arange(len(procs))
        bar_width = 0.6

        ax.bar(x_pos, comp_vals, bar_width, label='Computation', color='#4a90e2', edgecolor='black', alpha=0.9)
        ax.bar(x_pos, comm_vals, bar_width, bottom=comp_vals, label='Communication', color='#ff3333', edgecolor='black', alpha=0.9)

        ax.set_yscale('log')
        ax.yaxis.set_major_locator(ticker.LogLocator(base=10.0, numticks=15))
        ax.yaxis.set_minor_locator(ticker.LogLocator(base=10.0, subs=np.arange(0.1, 1.0, 0.1), numticks=15))
        ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda y, _: '{:g}'.format(y)))
        ax.yaxis.set_minor_formatter(ticker.NullFormatter()) 

        ax.set_ylabel('Time (ms) - Log Scale', fontsize=12)
        ax.set_xlabel('Number of Processes ($P$)', fontsize=12)
        ax.set_title(f'MPI Performance Breakdown (ms): {matrix}', fontsize=14, fontweight='bold')
        
        ax.set_xticks(x_pos)
        ax.set_xticklabels(procs)
        
        ax.legend(loc='upper right', frameon=True)
        ax.grid(True, which="major", linestyle='-', alpha=0.6)
        ax.grid(True, which="minor", linestyle='--', alpha=0.2)
        
        out_filename = f"Strong_MPI_CompVsComm_MS_{matrix}.png"
        save_path = os.path.join(OUTPUT_DIR, out_filename)
        
        plt.tight_layout()
        plt.savefig(save_path, dpi=300)
        plt.close()
        print(f"[PLOT] Generato (Strong): {out_filename}")



def plot_weak_scaling_comm_vs_comp_ms():
    print("\n--- Generazione Grafici Comm vs Comp (Weak - Dynamic Log) ---")
    
    filename = 'weak_scaling_all.csv' 
    filepath = os.path.join(RAW_DIR, filename)

    if not os.path.exists(filepath):
        print(f"[WARN] File raw Weak Scaling non trovato: {filepath}. Salto.")
        return

    df = pd.read_csv(filepath, dtype=str)
    cols = ['elapsed_time', 'comm_time', 'num_procs']
    for col in cols:
        df[col] = pd.to_numeric(df[col], errors='coerce')
    
    df = df.dropna(subset=cols)
    df['num_procs'] = df['num_procs'].astype(int)

    valid_procs = [1, 2, 4, 8, 16, 32, 64, 128]
    df = df[df['num_procs'].isin(valid_procs)]

    if df.empty:
        print("[ERR] Nessun dato valido Weak Scaling trovato.")
        return
    
    df['clean_matrix'] = df['matrix_name'].apply(
        lambda x: 'Synthetic' if 'synthetic' in str(x).lower() else os.path.basename(str(x)).replace('.mtx', '')
    )
    
    unique_matrices = df['clean_matrix'].unique()

    for matrix in unique_matrices:
        sub_df = df[df['clean_matrix'] == matrix].copy()
        if sub_df.empty: continue

        grouped = sub_df.groupby('num_procs').agg({
            'elapsed_time': 'mean',
            'comm_time': 'mean'
        }).reset_index().sort_values('num_procs')

        grouped['elapsed_time_ms'] = grouped['elapsed_time'] * 1000
        grouped['comm_time_ms'] = grouped['comm_time'] * 1000
        grouped['comp_time_ms'] = grouped['elapsed_time_ms'] - grouped['comm_time_ms']
        grouped['comp_time_ms'] = grouped['comp_time_ms'].clip(lower=0)

        procs = grouped['num_procs'].astype(str).tolist()
        comp_vals = grouped['comp_time_ms'].tolist()
        comm_vals = grouped['comm_time_ms'].tolist()
        
        fig, ax = plt.subplots(figsize=(10, 6))
        x_pos = np.arange(len(procs))
        bar_width = 0.6

        ax.bar(x_pos, comp_vals, bar_width, label='Computation', color='#4a90e2', edgecolor='black', alpha=0.9)
        ax.bar(x_pos, comm_vals, bar_width, bottom=comp_vals, label='Communication', color='#ff3333', edgecolor='black', alpha=0.9)

        ax.set_yscale('log')
        ax.yaxis.set_major_locator(ticker.LogLocator(base=10.0, numticks=15))
        ax.yaxis.set_minor_locator(ticker.LogLocator(base=10.0, subs=np.arange(0.1, 1.0, 0.1), numticks=15))
        ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda y, _: '{:g}'.format(y)))
        ax.yaxis.set_minor_formatter(ticker.NullFormatter()) 

        ax.set_ylabel('Time (ms) - Log Scale', fontsize=12)
        ax.set_xlabel('Number of Processes ($P$)', fontsize=12)
        ax.set_title(f'Weak Scaling - MPI Breakdown (ms): {matrix}', fontsize=14, fontweight='bold')
        
        ax.set_xticks(x_pos)
        ax.set_xticklabels(procs)
        
        ax.legend(loc='upper right', frameon=True)
        ax.grid(True, which="major", linestyle='-', alpha=0.6)
        ax.grid(True, which="minor", linestyle='--', alpha=0.2)
        
        out_filename = f"Weak_MPI_CompVsComm_MS_{matrix}.png"
        save_path = os.path.join(OUTPUT_DIR, out_filename)
        
        plt.tight_layout()
        plt.savefig(save_path, dpi=300)
        plt.close()
        print(f"[PLOT] Generato (Weak): {out_filename}")



def plot_mpi_gflops_scaling():
    """
    Genera il grafico dei Gflop/s vs Processi SOLO per Pure MPI.
    TIPO: Column Chart (Bar Plot).
    """
    print("\n--- Generazione Grafico Gflop/s (Bar Chart - Pure MPI) ---")
    
    files = [f for f in os.listdir(INPUT_DIR) if f.startswith('summary_') and f.endswith('.csv')]
    
    for filename in files:
        filepath = os.path.join(INPUT_DIR, filename)
        df = pd.read_csv(filepath)
        
        df_mpi = df[df['mode'].astype(str).str.contains("Pure MPI", case=False)].copy()
        if df_mpi.empty: continue
        
        matrix_name = filename.replace('summary_', '').replace('.csv', '').replace('_combined', '')
        
        gflops_col = None
        for col in df_mpi.columns:
            if 'gflops' in col.lower():
                gflops_col = col
                break
        
        if gflops_col is None:
            if 'nnz' in df_mpi.columns and 'p90_time' in df_mpi.columns:
                print(f" -> Calcolo Gflops manuale per {matrix_name}")
                df_mpi['calc_gflops'] = (2 * df_mpi['nnz']) / (df_mpi['p90_time'] * 1e9)
                gflops_col = 'calc_gflops'
            else:
                continue

        df_mpi = df_mpi.sort_values('num_procs')

        fig, ax = plt.subplots(figsize=(8, 6))
        
        sns.barplot(data=df_mpi, x='num_procs', y=gflops_col, 
                    color='#d62728', edgecolor='black', alpha=0.8, ax=ax)

        for container in ax.containers:
            ax.bar_label(container, fmt='%.2f', padding=3, fontsize=10)

        ax.set_ylabel("Gflop/s", fontsize=12) 
        ax.set_xlabel("Number of Processes ($P$)", fontsize=12)
        ax.set_title(f"Performance (Gflop/s): {matrix_name}\nPure MPI Only", fontsize=14, fontweight='bold')
        
        ax.grid(axis='y', linestyle='--', alpha=0.6)
        
        out_filename = f"GFLOPS_Bar_PureMPI_{matrix_name}.png"
        save_path = os.path.join(OUTPUT_DIR, out_filename)
        
        plt.tight_layout()
        plt.savefig(save_path, dpi=300)
        plt.close()
        print(f"[PLOT] Gflops Bar Chart Generato: {out_filename}")



def main():
    print("=== INIZIO ANALISI ===")
    
    files = [f for f in os.listdir(INPUT_DIR) if f.startswith('summary_') and f.endswith('.csv')]
    for f in files:
        plot_strong_scaling_single(f)
        plot_weak_scaling_comparison(f)
    
    plot_combined_efficiency_mpi_only()

    plot_mpi_comm_vs_comp_stacked_ms_clean()

    plot_weak_scaling_comm_vs_comp_ms()

    plot_mpi_gflops_scaling()
    
    print("=== COMPLETATO ===")

if __name__ == "__main__":
    main()
