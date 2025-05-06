import pandas as pd
import matplotlib.pyplot as plt
import glob
import os
import re
from typing import Tuple, List, Optional
from matplotlib.axes import Axes

MD_TEMPLATE = """# Data: `{0}`, {1}, {2}
## Without Parallel Computing
### PCC
```shell
Result: {3}
Computation Time: {4:.6f}
```
### SSD
```shell
Result: {5}
Computation Time: {6:.6f}
```

## With Parallel Computing
### PCC, Threads: {7}
```shell
Result: {8}
Computation Time: {9:.6f}
```
### SSD, Threads: {10}
```shell
Result: {11}
Computation Time: {12:.6f}
```

# Summary
### SSD Analysis
Slowest vs fastest speedup: {13:.2f}x
- Fastest: SSD (Threads: {14}, Time: {15:.6f}s)
- Slowest: SSD (Threads: {16}, Time: {17:.6f}s)

### PCC Analysis
Slowest vs fastest speedup: {18:.2f}x
- Fastest: PCC (Threads: {19}, Time: {20:.6f}s)
- Slowest: PCC (Threads: {21}, Time: {22:.6f}s)
"""


def parse_filename(f: str) -> Tuple[str, Optional[str], Optional[str]]:
    filename: str = os.path.splitext(os.path.basename(f))[0]
    match = re.match(r"^(.*?)(?:_(\d+x\d+)_(\d+x\d+))$", filename)
    return match.groups() if match else (filename, None, None)


def generate_markdown(f: str) -> None:
    name, s_size, t_size = parse_filename(f)
    df: pd.DataFrame = pd.read_csv(f)
    if not all(
        c in df.columns for c in ["methods", "threads", "best_positions", "cost_time"]
    ):
        return
    md_file: str = "outputs/{0}_{1}_{2}.md".format(name, s_size, t_size)

    pcc_1 = df[(df["methods"] == "PCC") & (df["threads"] == 1)]
    ssd_1 = df[(df["methods"] == "SSD") & (df["threads"] == 1)]
    max_threads: int = df["threads"].max()
    pcc_max = df[(df["methods"] == "PCC") & (df["threads"] == max_threads)]
    ssd_max = df[(df["methods"] == "SSD") & (df["threads"] == max_threads)]

    pcc_1_pos, pcc_1_time = (
        (pcc_1.iloc[0]["best_positions"].strip(), pcc_1.iloc[0]["cost_time"])
        if not pcc_1.empty
        else ("N/A", 0.0)
    )
    ssd_1_pos, ssd_1_time = (
        (ssd_1.iloc[0]["best_positions"].strip(), ssd_1.iloc[0]["cost_time"])
        if not ssd_1.empty
        else ("N/A", 0.0)
    )
    pcc_max_pos, pcc_max_time = (
        (pcc_max.iloc[0]["best_positions"].strip(), pcc_max.iloc[0]["cost_time"])
        if not pcc_max.empty
        else ("N/A", 0.0)
    )
    ssd_max_pos, ssd_max_time = (
        (ssd_max.iloc[0]["best_positions"].strip(), ssd_max.iloc[0]["cost_time"])
        if not ssd_max.empty
        else ("N/A", 0.0)
    )

    ssd_times = df[df["methods"] == "SSD"]["cost_time"].tolist()
    pcc_times = df[df["methods"] == "PCC"]["cost_time"].tolist()

    ssd_speedup, ssd_fast, ssd_fast_time, ssd_slow, ssd_slow_time = (
        (0.0, 0, 0.0, 0, 0.0)
        if not ssd_times
        else (
            max(ssd_times) / min(ssd_times) if min(ssd_times) > 0 else float("inf"),
            df[(df["methods"] == "SSD") & (df["cost_time"] == min(ssd_times))].iloc[0][
                "threads"
            ],
            min(ssd_times),
            df[(df["methods"] == "SSD") & (df["cost_time"] == max(ssd_times))].iloc[0][
                "threads"
            ],
            max(ssd_times),
        )
    )
    pcc_speedup, pcc_fast, pcc_fast_time, pcc_slow, pcc_slow_time = (
        (0.0, 0, 0.0, 0, 0.0)
        if not pcc_times
        else (
            max(pcc_times) / min(pcc_times) if min(pcc_times) > 0 else float("inf"),
            df[(df["methods"] == "PCC") & (df["cost_time"] == min(pcc_times))].iloc[0][
                "threads"
            ],
            min(pcc_times),
            df[(df["methods"] == "PCC") & (df["cost_time"] == max(pcc_times))].iloc[0][
                "threads"
            ],
            max(pcc_times),
        )
    )

    with open(md_file, "w") as md:
        md.write(
            MD_TEMPLATE.format(
                name,
                s_size,
                t_size,
                pcc_1_pos,
                pcc_1_time,
                ssd_1_pos,
                ssd_1_time,
                max_threads,
                pcc_max_pos,
                pcc_max_time,
                max_threads,
                ssd_max_pos,
                ssd_max_time,
                ssd_speedup,
                ssd_fast,
                ssd_fast_time,
                ssd_slow,
                ssd_slow_time,
                pcc_speedup,
                pcc_fast,
                pcc_fast_time,
                pcc_slow,
                pcc_slow_time,
            )
        )
    print("Generated: {0}".format(md_file))


def plot_csv_data(ax: Axes, f: str) -> None:
    name, s_size, t_size = parse_filename(f)
    df: pd.DataFrame = pd.read_csv(f)
    if not all(c in df.columns for c in ["methods", "threads", "cost_time"]):
        ax.set_title("Invalid: {0}".format(name))
        ax.grid(True, linestyle="--", alpha=0.7)
        return
    times: List[float] = []
    colors: List[str] = ["blue", "orange"]  # PCC: blue, SSD: orange
    for m, color in zip(["PCC", "SSD"], colors):
        d: pd.DataFrame = df[df["methods"] == m][["threads", "cost_time"]].sort_values(
            "threads"
        )
        if d.empty:
            continue
        ax.plot(
            d["threads"],
            d["cost_time"],
            marker="o",
            label=m,
            linewidth=1.5,
            markersize=4,
            color=color,
        )
        times.extend(d["cost_time"].tolist())
    ax.set_title(
        "{0} ({1}, {2})".format(name, s_size, t_size) if s_size else "{0}".format(name),
        fontsize=10,
        pad=5,
    )
    ax.set_xlabel("Threads", fontsize=8)
    ax.set_ylabel("Time (s)", fontsize=8)
    ax.set_ylim(min(times) * 0.95, max(times) * 1.05) if times else None
    ax.grid(True, linestyle="--", alpha=0.7)
    ax.legend(fontsize=8)
    ax.tick_params(axis="both", labelsize=7)


def main() -> None:
    os.makedirs("outputs", exist_ok=True)
    csv_files: List[str] = sorted(glob.glob("outputs/*.csv"))
    if not csv_files:
        print("No CSVs in outputs/")
        return
    n: int = len(csv_files)
    print("Found {0} CSVs: {1}".format(n, csv_files))
    fig, axes = plt.subplots(1, n, figsize=(5 * n, 5))
    axes = [axes] if n == 1 else axes
    for i, f in enumerate(csv_files):
        plot_csv_data(axes[i], f)
        generate_markdown(f)
    fig.suptitle("Threads vs Time", fontsize=14, y=1.02)
    plt.tight_layout()
    plt.savefig("outputs/combined_threads_vs_time.png", dpi=300, bbox_inches="tight")
    plt.close()
    print("Generated: outputs/combined_threads_vs_time.png")


if __name__ == "__main__":
    main()
