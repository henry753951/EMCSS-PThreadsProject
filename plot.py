import pandas as pd
import matplotlib.pyplot as plt
import glob
import os
import re
from typing import Tuple, List, Optional
from matplotlib.axes import Axes


def parse_filename(f: str) -> Tuple[str, Optional[str], Optional[str]]:
    filename: str = os.path.splitext(os.path.basename(f))[0]
    match = re.match(r"^(.*?)(?:_(\d+x\d+)_(\d+x\d+))$", filename)
    if match:
        return match.groups()
    return filename, None, None


def plot_csv_data(ax: Axes, f: str) -> None:
    name, s_size, t_size = parse_filename(f)
    df: pd.DataFrame = pd.read_csv(f)
    if not all(c in df.columns for c in ["methods", "threads", "cost_time"]):
        ax.set_title(f"Invalid: {name}")
        ax.grid(True, linestyle="--", alpha=0.7)
        return
    times: List[float] = []
    for m in df["methods"].unique():
        d: pd.DataFrame = df[df["methods"] == m][["threads", "cost_time"]].sort_values(
            "threads"
        )
        ax.plot(
            d["threads"],
            d["cost_time"],
            marker="o",
            label=m,
            linewidth=1.5,
            markersize=4,
        )
        times.extend(d["cost_time"].tolist())
    ax.set_title(f"{name} ({s_size}, {t_size})" if s_size else name, fontsize=10, pad=5)
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
    fig, axes = plt.subplots(1, n, figsize=(5 * n, 5))
    axes = [axes] if n == 1 else axes
    [plot_csv_data(axes[i], f) for i, f in enumerate(csv_files)]
    fig.suptitle("Threads vs Time", fontsize=14, y=1.02)
    plt.tight_layout()
    plt.savefig("outputs/combined_threads_vs_time.png", dpi=300, bbox_inches="tight")
    plt.close()
    print("Generated: outputs/combined_threads_vs_time.png")


if __name__ == "__main__":
    main()
