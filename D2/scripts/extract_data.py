import pandas as pd
import os

# --- CONFIGURAZIONE ---
# Percorso dello script corrente (.../scripts)
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

# Percorso della cartella results (.../results)
RESULTS_DIR = os.path.join(CURRENT_DIR, '../results')

# MODIFICA: La cartella di output ora è dentro 'results/tables'
OUTPUT_DIR = os.path.join(RESULTS_DIR, 'tables')

# Crea la cartella se non esiste
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

def parse_summary_metrics(filename, mode_label):
    """
    Legge un file di output MPI e cerca i blocchi di riepilogo statistico.
    Ritorna una lista di dizionari con i dati trovati.
    """
    filepath = os.path.join(RESULTS_DIR, filename)
    if not os.path.exists(filepath):
        print(f"ATTENZIONE: File non trovato: {filepath}")
        return []

    print(f"Analisi file: {filename}...")
    extracted_rows = []
    
    # Intestazione specifica che indica l'inizio del blocco di riepilogo nel file log
    SUMMARY_HEADER_START = "Matrix_Name,Num_Processes,Ghost_Entries_Min"
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
        
    for i, line in enumerate(lines):
        line = line.strip()
        
        # Se troviamo l'intestazione, leggiamo la riga successiva
        if line.startswith(SUMMARY_HEADER_START):
            if i + 1 < len(lines):
                data_line = lines[i+1].strip()
                if data_line:
                    parts = data_line.split(',')
                    try:
                        # Puliamo il nome della matrice (rimuoviamo path ../data/)
                        raw_name = parts[0]
                        clean_name = os.path.basename(raw_name)

                        record = {
                            'mode': mode_label,
                            'matrix_name': clean_name,
                            'num_procs': int(parts[1]),
                            'ghost_min': int(parts[2]),
                            'ghost_avg': float(parts[3]),
                            'ghost_max': int(parts[4]),
                            'load_imbalance': float(parts[5]),
                            'p90_time': float(parts[6]),
                            'total_gflops': float(parts[7])
                        }
                        extracted_rows.append(record)
                    except (ValueError, IndexError):
                        pass

    return extracted_rows

def main():
    # 1. RACCOLTA DATI
    all_data = []
    
    # Carichiamo tutti i file disponibili
    all_data.extend(parse_summary_metrics('strong_scaling_hybrid.csv', 'Hybrid (MPI+OMP)'))
    all_data.extend(parse_summary_metrics('strong_scaling_all.csv', 'Pure MPI'))
    all_data.extend(parse_summary_metrics('weak_scaling_hybrid.csv', 'Hybrid (MPI+OMP)'))
    all_data.extend(parse_summary_metrics('weak_scaling_all.csv', 'Pure MPI'))

    if not all_data:
        print("Nessun dato trovato. Verifica i file nella cartella results.")
        return

    # Creiamo un DataFrame pandas generale
    df = pd.DataFrame(all_data)

    # Definiamo l'ordine delle colonne per i file di output
    cols_order = ['matrix_name', 'mode', 'num_procs', 'total_gflops', 'p90_time', 'load_imbalance', 'ghost_avg']

    print(f"\n--- INIZIO SALVATAGGIO IN: {OUTPUT_DIR} ---")

    # 2. GESTIONE MATRICI SINTETICHE (WEAK SCALING) -> FILE UNICO
    # Filtriamo tutte le righe che contengono "synthetic" nel nome
    synthetic_df = df[df['matrix_name'].str.contains('synthetic', case=False, na=False)]
    
    if not synthetic_df.empty:
        # Ordiniamo per modalità e poi per numero di processori
        synthetic_df = synthetic_df.sort_values(by=['mode', 'num_procs'])
        # Riordiniamo le colonne
        synthetic_df = synthetic_df[cols_order]
        
        filename_syn = "summary_synthetic_combined.csv"
        path_syn = os.path.join(OUTPUT_DIR, filename_syn)
        synthetic_df.to_csv(path_syn, index=False)
        print(f"[OK] Salvato file combinato SINTETICO: {filename_syn}")

    # 3. GESTIONE MATRICI REALI (STRONG SCALING) -> FILE SEPARATI
    # Filtriamo le righe che NON sono sintetiche
    real_df = df[~df['matrix_name'].str.contains('synthetic', case=False, na=False)]
    
    if not real_df.empty:
        # Troviamo i nomi unici delle matrici reali (es. torso1.mtx, bcsstk14.mtx)
        unique_matrices = real_df['matrix_name'].unique()

        for matrix in unique_matrices:
            # Prendiamo solo i dati di QUESTA matrice
            sub_df = real_df[real_df['matrix_name'] == matrix].copy()
            
            # Ordiniamo
            sub_df = sub_df.sort_values(by=['mode', 'num_procs'])
            sub_df = sub_df[cols_order]
            
            # Puliamo il nome file (togliamo .mtx)
            safe_name = matrix.replace('.mtx', '')
            filename_real = f"summary_{safe_name}.csv"
            path_real = os.path.join(OUTPUT_DIR, filename_real)
            
            sub_df.to_csv(path_real, index=False)
            print(f"[OK] Salvato file MATRICE REALE: {filename_real}")
    else:
        print("Nessuna matrice reale trovata nei log.")

    print(f"\nOperazione completata. Controlla la cartella: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()