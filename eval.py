#!/usr/bin/env python

import matplotlib.pyplot as plt
import subprocess
import argparse
from pathlib import Path
import math
import numpy as np

def run_experiment(output_file, build_dir):
    # The number of threads is not currently used, it's just here in case you want to parallelize your code.
    for threads in [1, 2, 4, 8, 12, 16]:
        for size in np.logspace(4, 8, num=16):
            print("Measuring p=" + str(threads) + " n=" + str(size))
            executable = Path(build_dir) / "Sorter"
            returncode = subprocess.call([executable, str(size), str(threads)], stdout=output_file)
            if returncode != 0:
                print("Program crashed")

def make_plot(result_file):
    plots = dict()
    maxDuration = 0
    for line in result_file:
        if line.startswith("RESULT"):
            line = line[len("RESULT "):].strip()
            parts = line.split()
            measurement = {}
            for part in parts:
                key, value = part.split('=')
                measurement[key] = value

            n = int(round(int(measurement["n"]), -1))
            t = int(measurement["t"])
            name = measurement["name"]
            durationNanoseconds = int(measurement["durationNanoseconds"])
            constructorNanoseconds = int(measurement["constructorNanoseconds"])

            if t not in plots:
                plots[t] = dict()
            if name not in plots[t]:
                plots[t][name] = list()
                plots[t][name + " (constructor)"] = list()
            plots[t][name].append((n, durationNanoseconds / n))
            plots[t][name + " (constructor)"].append((n, constructorNanoseconds / n))
            maxDuration = max(maxDuration, max(durationNanoseconds, constructorNanoseconds))

        # Create a grid of subplots; always get a flat list of axes
    num = len(plots)
    cols = 2 if num > 2 else 1               # 1 Spalte für bis zu 2 Plots, sonst 2 Spalten
    rows = math.ceil(num / cols)

    fig, axs = plt.subplots(
        rows, cols,
        squeeze=False,                        # immer 2D-Array zurück
        figsize=(8*cols, 4*rows),             # mehr Platz pro Subplot
        constrained_layout=True               # sorgt für „entspanntere“ Abstände
    )
    axs = axs.flatten()

    # Iterate deterministically over thread counts
    for i, t in enumerate(sorted(plots.keys())):
        ax = axs[i]

        # Only use base algorithm names (exclude the constructor variants)
        base_names = [k for k in plots[t].keys() if not k.endswith(" (constructor)")]

        # Plot each base and its constructor series
        for base in base_names:
            # sort by n to get nice lines
            xs, ys = zip(*sorted(plots[t][base], key=lambda p: p[0]))
            ax.plot(xs, ys, label=base, marker='x')

            xs2, ys2 = zip(*sorted(plots[t][base + " (constructor)"], key=lambda p: p[0]))
            ax.plot(xs2, ys2, label=base + " (constructor)", marker='+')

        if len(plots) > 1:
            ax.set_title(f"#p={t}")
        ax.set_xscale('log')
        ax.set(xlabel='n', ylabel='Running time per element (ns)')
        ax.legend()

    plt.tight_layout()
    plt.savefig("plot.pdf")


def main():
    parser = argparse.ArgumentParser(description='evaluation tools')
    # subcommands run and plot
    subparsers = parser.add_subparsers(dest='command')
    run_parser = subparsers.add_parser('run')
    run_parser.add_argument('output_file', type=argparse.FileType('w'), help='output file')
    run_parser.add_argument("--build-dir", default="build", help="build directory")
    plot_parser = subparsers.add_parser('plot')
    plot_parser.add_argument('result_file', type=argparse.FileType('r'), help='result file')
    args = parser.parse_args()
    if args.command == 'run':
        run_experiment(args.output_file, args.build_dir)
    elif args.command == 'plot':
        make_plot(args.result_file)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
