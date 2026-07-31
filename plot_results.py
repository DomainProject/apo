import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import pandas as pd

df = pd.read_csv("results.csv")

actors = df["num_actors"].tolist()
total  = df["time"].tolist()
ddm    = df["time_ddm"].tolist()
pct    = [d / t * 100 for d, t in zip(ddm, total)]

fig, ax1 = plt.subplots(figsize=(9, 5))

x = np.arange(len(actors))
w = 0.35

ax1.bar(x - w/2, total, w, label="Total Execution Time", color="#B5D4F4", edgecolor="#378ADD", linewidth=0.8)
ax1.bar(x + w/2, ddm,   w, label="DDM Execution Time",   color="#F5C4B3", edgecolor="#D85A30", linewidth=0.8)
ax1.set_yscale("log")
ax1.set_ylabel("seconds", color="#444441")
ax1.set_xlabel("number of actors")
ax1.set_xticks(x)
ax1.set_xticklabels(actors)
ax1.yaxis.set_major_formatter(ticker.FuncFormatter(lambda v, _: f"{v:g}s"))
ax1.tick_params(axis="y", labelcolor="#444441")

ax2 = ax1.twinx()
ax2.plot(x, pct, color="#1D9E75", linestyle="--", linewidth=2,
         marker="o", markersize=5, label="% DDM / Total")
ax2.set_ylim(0, 100)
ax2.set_ylabel("% DDM", color="#1D9E75")
ax2.tick_params(axis="y", labelcolor="#1D9E75")
ax2.yaxis.set_major_formatter(ticker.FuncFormatter(lambda v, _: f"{v:.0f}%"))

lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines1 + lines2, labels1 + labels2, fontsize=10, framealpha=0.8)

plt.tight_layout()
plt.title("DDM overhead vs. number of actors (4 CPUs, 2 GPUs, 1 FPGA) simulating 100 seconds")
plt.savefig("results.png", dpi=150, bbox_inches="tight")
plt.show()


df2 = pd.read_csv("results_hardware.csv")

# Creazione label asse X: "CPU-GPU-FPGA"
df2['label'] = (df2['num_cpus'].astype(str) + "C-" +
                df2['num_gpus'].astype(str) + "G-" +
                df2['num_fpgas'].astype(str) + "F")

# Calcolo percentuale gestendo i NaN (timeout)
df2['pct'] = (df2['time_ddm'] / df2['time']) * 100

# Setup figura più larga per i 64 campioni
fig2, ax2_1 = plt.subplots(figsize=(15, 6))
x2 = np.arange(len(df2))
w2 = 0.4

# Barre Tempo
ax2_1.bar(x2 - w2/2, df2['time'], w2, label="Total Time", color="#B5D4F4", edgecolor="#378ADD", linewidth=0.5)
ax2_1.bar(x2 + w2/2, df2['time_ddm'], w2, label="DDM Time", color="#F5C4B3", edgecolor="#D85A30", linewidth=0.5)

# Marker per i Timeout (dove time_ddm è assente)
timeout_mask = df2['time_ddm'].isna()
ax2_1.scatter(x2[timeout_mask], df2.loc[timeout_mask, 'time'], color='red', marker='x', s=30, label="Timeout (300s)")

ax2_1.set_yscale("log")
ax2_1.set_ylabel("seconds", color="#444441")
ax2_1.set_xlabel("Hardware Configuration (CPUs-GPUs-FPGAs)")
ax2_1.set_xticks(x2)
ax2_1.set_xticklabels(df2['label'], rotation=90, fontsize=7)
ax2_1.yaxis.set_major_formatter(ticker.FuncFormatter(lambda v, _: f"{v:g}s"))
ax2_1.grid(axis='y', linestyle=':', alpha=0.5)

# Secondo asse per la percentuale
ax2_2 = ax2_1.twinx()
ax2_2.plot(x2, df2['pct'], color="#1D9E75", linestyle="-", linewidth=1, marker=".", alpha=0.7, label="% DDM Overhead")
ax2_2.set_ylim(0, 100)
ax2_2.set_ylabel("% DDM", color="#1D9E75")
ax2_2.tick_params(axis="y", labelcolor="#1D9E75")
ax2_2.yaxis.set_major_formatter(ticker.PercentFormatter())

lines_a, labels_a = ax2_1.get_legend_handles_labels()
lines_b, labels_b = ax2_2.get_legend_handles_labels()
ax2_1.legend(lines_a + lines_b, labels_a + labels_b, loc='upper left', ncol=4, fontsize=9)

plt.title("Execution time across hardware scaling")
plt.tight_layout()
plt.savefig("hardware_scaling.png", dpi=150, bbox_inches="tight")
plt.show()
