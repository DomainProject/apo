#!/usr/bin/env python3
import os
import re
import sys

def extract_data(file_path):
    """
    Legge il file specificato e estrae i datapoint individuando il valore WT come ascissa e
    il valore TH (com per msec) come ordinata. Se il campo "MAX TH" è diverso da 0 e non è
    ancora stato registrato un valore di riferimento, lo salva come riferimento.
    """
    data_points = []
    ref_val = None
    # Apertura del file in modalità lettura
    with open(file_path, "r") as f:
        print(f'Processing {file_path}...')
        for line in f:
            # Utilizzo di una regex per trovare le parti interessanti:
            # WT <valore> ... TH (com per msec) <valore> ... MAX TH <valore>
            m = re.search(r'WT\s+([0-9.]+)\sGVT\s+(\d+.?\d*).*?TH\s+\(com per msec\)\s+([0-9.]+).*?MAX TH\s+([0-9.]+)', line)
            if m:
                wt_val = m.group(1)
                vt_val = m.group(2)
                th_val     = float(m.group(3))
                max_th_val = float(m.group(4))
                end_sim_time = float(wt_val)
                print(wt_val, th_val, max_th_val)
                if max_th_val > 0:
                    th_val = min(th_val,max_th_val)
                data_points.append((wt_val, th_val))
                # Se non abbiamo ancora registrato il valore di riferimento e il valore MAX TH è diverso da 0
                if ref_val is None and float(max_th_val) != 0:
                    ref_val = float(max_th_val)
    return data_points, ref_val, end_sim_time

import os


def main():
    # Verifica che siano stati passati almeno uno o più file come argomenti
    if len(sys.argv) < 2:
        print("Usage: {} file1 [file2 ...]".format(sys.argv[0]))
        sys.exit(1)

    input_files = sys.argv[1:]
    overall_ref = 0
    min_end_sim_time = float('inf')
    dat_files = []  # Lista di tuple (nome_file_dat, titolo) per il comando plot

    # Elaborazione di ogni file
    for file_path in input_files:
        data, ref_val, end_sim_time = extract_data(file_path)
        # Se non è ancora stato assegnato un valore di riferimento globale, si utilizza quello trovato
        if end_sim_time is not None:
            min_end_sim_time = min(min_end_sim_time, end_sim_time)
        base_name = os.path.splitext(os.path.basename(file_path))[0]
        dat_filename = base_name + ".dat"
        dat_files.append((dat_filename, base_name))
        # Scrittura del file .dat con i datapoint (ascissa e ordinata separati da uno spazio)
        with open(dat_filename, "w") as out:
            for wt, th in data:
                if th > overall_ref: overall_ref = th
                out.write(f"{wt} {th}\n")

    # Generazione del file di script per gnuplot
    plt_filename = "plot.plt"
    with open(plt_filename, "w") as plt_file:
        print(os.getcwd().split('/')[-1] + "\n")
        plt_file.write("set terminal pdfcairo enhanced color size 5in,2.5in font \"Linux Libertine, 12\"\n")
        plt_file.write(f"set output '{os.getcwd().split('/')[-1]}.pdf'\n")
        plt_file.write("set xlabel 'Simulation Time'\n")
        plt_file.write("set ylabel 'Throughput (task/msec)'\n")
        plt_file.write("set grid y\n")
        plt_file.write("set samples 5000\n")
        plt_file.write(f"set xrange [0:{min_end_sim_time}]\n")
        plt_file.write(f"set yrange [0:{overall_ref*1.1}]\n")
        plt_file.write("set key center below\n")
        # Se è stato trovato un valore di riferimento diverso da 0, aggiunge una linea orizzontale
        if overall_ref is not None:
            plt_file.write(f"set arrow from graph 0, first {overall_ref} to graph 1, first {overall_ref} nohead\n")
        # Costruzione del comando di plot che include ogni file .dat generato
        plot_cmd = "plot "
        for dat, title in dat_files:
            plot_cmd += f"'{dat}' using 1:2 smooth unique with lines title '{title}', "
        # Rimozione della virgola finale e aggiunta del comando di pausa
        plot_cmd = plot_cmd.rstrip(", ") + "\n"
        plt_file.write(plot_cmd)

if __name__ == "__main__":
    main()
