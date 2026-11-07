"""Plot ConvoScale timing CSVs: time, speedup, and efficiency."""

import argparse
import csv
import sys
from collections import defaultdict

COLUMNS = [
    "dim",
    "mode",
    "filter",
    "width",
    "height",
    "depth",
    "kernel",
    "threads",
    "schedule",
    "chunk",
    "tile",
    "seconds",
    "repeat",
]
MATCH = ["dim", "filter", "width", "height", "depth", "kernel"]


def read_rows(path):
    with open(path, newline="") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None:
            raise SystemExit(f"{path}: missing header")
        missing = [name for name in COLUMNS if name not in reader.fieldnames]
        if missing:
            raise SystemExit(f"{path}: missing columns {', '.join(missing)}")
        rows = []
        for row in reader:
            item = {name: row[name].strip() for name in COLUMNS}
            item["seconds"] = float(item["seconds"])
            rows.append(item)
    if not rows:
        raise SystemExit(f"{path}: no data rows")
    return rows


def average(rows):
    groups = defaultdict(list)
    for row in rows:
        key = tuple(row[name] for name in COLUMNS if name not in ("seconds", "repeat"))
        groups[key].append(row["seconds"])
    averaged = []
    for key, samples in groups.items():
        item = dict(zip([name for name in COLUMNS if name not in ("seconds", "repeat")], key))
        item["seconds"] = sum(samples) / len(samples)
        averaged.append(item)
    return averaged


def sequential_index(rows):
    index = {}
    for row in rows:
        if row["mode"] != "seq" and row["threads"] not in ("1", "na"):
            continue
        index[tuple(row[name] for name in MATCH)] = row["seconds"]
    return index


def attach_speedup(parallel_rows, baseline):
    for row in parallel_rows:
        key = tuple(row[name] for name in MATCH)
        seq = baseline.get(key)
        if seq is None or seq == 0.0 or row["threads"] in ("", "na"):
            row["speedup"] = None
            row["efficiency"] = None
            continue
        threads = float(row["threads"])
        row["speedup"] = seq / row["seconds"]
        row["efficiency"] = row["speedup"] / threads if threads else None


def series_key(row, x_name):
    parts = []
    for name in ("dim", "mode", "filter", "schedule", "kernel", "width", "tile"):
        if name == x_name:
            continue
        parts.append(f"{name}={row[name]}")
    return " ".join(parts)


def write_plot(path, rows, x_name, y_name, title):
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    grouped = defaultdict(list)
    for row in rows:
        if row.get(y_name) is None:
            continue
        try:
            x_value = float(row[x_name])
        except ValueError:
            continue
        grouped[series_key(row, x_name)].append((x_value, row[y_name]))

    if not grouped:
        print(f"skip {path}: no numeric {x_name}/{y_name} rows")
        return

    figure, axis = plt.subplots(figsize=(8, 4.5))
    for label, points in sorted(grouped.items()):
        points.sort()
        axis.plot([p[0] for p in points], [p[1] for p in points], marker="o", label=label)
    axis.set_xlabel(x_name)
    axis.set_ylabel(y_name)
    axis.set_title(title)
    axis.grid(True, alpha=0.3)
    axis.legend(fontsize=7)
    figure.tight_layout()
    figure.savefig(path, dpi=120)
    plt.close(figure)
    print(path)


def main():
    parser = argparse.ArgumentParser(description="Plot ConvoScale benchmark CSVs")
    parser.add_argument("csv", help="parallel timing CSV")
    parser.add_argument("--sequential", help="sequential CSV used as the speedup denominator")
    parser.add_argument("--x", default="threads", choices=["threads", "width", "kernel", "tile"])
    parser.add_argument("--out", default="results/plots", help="directory for PNG files")
    args = parser.parse_args()

    rows = average(read_rows(args.csv))
    baseline_rows = average(read_rows(args.sequential)) if args.sequential else rows
    attach_speedup(rows, sequential_index(baseline_rows))

    try:
        import matplotlib  # noqa: F401
    except ImportError:
        print("matplotlib is not installed; CSV was read successfully", file=sys.stderr)
        return 0

    import os

    os.makedirs(args.out, exist_ok=True)
    stem = os.path.splitext(os.path.basename(args.csv))[0]
    write_plot(os.path.join(args.out, f"{stem}_time.png"), rows, args.x, "seconds", f"Time vs {args.x}")
    write_plot(
        os.path.join(args.out, f"{stem}_speedup.png"),
        rows,
        args.x,
        "speedup",
        f"Speedup vs {args.x}",
    )
    write_plot(
        os.path.join(args.out, f"{stem}_efficiency.png"),
        rows,
        args.x,
        "efficiency",
        f"Efficiency vs {args.x}",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
