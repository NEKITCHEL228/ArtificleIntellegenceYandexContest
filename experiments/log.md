# AI Competition Experiment Log

## Baseline (Iteration 0)
- **Timestamp**: 2026-11-23 12:00:00 UTC
- **Git Commit**: `baseline`
- **Hypothesis**: Baseline initial model implementation
- **CV Metric**: 1.2500 (Loss)
- **Reviewer Verdict**: `APPROVE`
- **Status**: `BENCHMARK`

---

## Iteration 1 (Task 2: 2. Миллионы путей)
- **Timestamp**: 2026-11-23 13:08:00 UTC
- **Task ID**: `2_task`
- **Hypothesis**: HYP-02: Goal-directed A* with Dial's Algorithm (4 circular ring buckets `f & 3`), 1-cell border padding, O(1) adjacency resolution, and POSIX fast I/O (`getchar_unlocked` / `putchar_unlocked`).
- **Benchmark Result**:
  - Full $2048 \times 2048$ grid: **9.83x speedup** (from 63.4 qps to 622.6 qps, 500 queries processed in 0.803s vs 7.892s).
  - Open $1000 \times 1000$ grid: **9.45x speedup** (from 305.9 qps to 2891.8 qps).
  - Labyrinth/maze grid: **3.75x speedup** (from 22.4k qps to 84.1k qps).
  - Exact match verification: **0 mismatches / 2000 queries (100% precision)**.
- **Reviewer Verdict**: `APPROVE` (admissible and consistent heuristic, bounds-safe padding, zero memory leaks, interactive flush protocol preserved).
- **Status**: `COMMITTED`

---

## Iteration 2 (Task 2: 2. Миллионы путей)
- **Timestamp**: 2026-11-23 13:58:00 UTC
- **Task ID**: `2_task`
- **Hypothesis**: HYP-01: ALT Algorithm (A* with Landmarks & Triangle Inequality $|d(u, L) - d(v, L)| \le \text{dist}(u, v)$), Farthest Point Sampling (FPS) with $K=16$ landmarks, $O(1)$ exact shortcut when $\max_i |d(s, L_i) - d(t, L_i)| == \min_i (d(s, L_i) + d(t, L_i))$, and adaptive dual-path search dispatch (pure register Manhattan Dial when no landmark bound exceeds Manhattan; 2-active-landmark ALT when detours/obstacles detected).
- **Benchmark Result**:
  - Open Grid $800 \times 800$: **1.48x speedup** (18,892.8 qps vs 12,755.2 qps).
  - Dense Obstacles $800 \times 800$ (25% blocked): **1.19x speedup** (6,201.2 qps vs 5,218.4 qps).
  - Maze / Labyrinth $801 \times 801$: **4.42x speedup** (4,875.3 qps vs 1,102.2 qps, total query time dropped from 4.536s to 1.026s).
  - Verification across all 20,000 queries in 4 distinct topologies: **100% exact match (0 mismatches)**.
- **Reviewer Verdict**: `APPROVE` (provably admissible and consistent triangle inequality heuristic, clean MaxMin FPS initialization in $\sim 0.15$s, memory $\sim 268$MB well under 2048MB limit, C++20 compliant, immediate interactive `fflush(stdout)` preserved).
- **Status**: `COMMITTED`
