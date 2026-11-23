# AGENTS.md

## Multi-Agent Architecture for AI Competitions

This document details the multi-agent orchestration, communication protocols, context isolation boundaries, and model routing policies established for competitive programming in the "Artificial Intelligence" track.

---

## 1. Interaction Topology

The multi-agent system uses a **Hub-and-Spoke** topology where the **Orchestrator** acts as the central router and arbiter.

```
                   +-------------------+
                   |       USER        |
                   +-------------------+
                             ^
                             | (Ideas, Confirmations)
                             v
                   +-------------------+
                   |   ORCHESTRATOR    |  <-- Tier 1 Reasoning
                   +-------------------+
                     /       |       \
                    /        |        \
  (Isolated Context)         |         (Spec & Perf Review)
         v                   v                   v
+-----------------+  +---------------+  +-----------------+
|  CODE-REVIEWER  |  |     CODER     |  |    OPTIMIZER    |
| (Tier 1 Reason) |  | (Tier 2 Code) |  | (Tier 1 Reason) |
+-----------------+  +---------------+  +-----------------+
         |                   ^
   (Higher Weight)           | (Technical Spec ТЗ)
         |                   |
         +----[ Veto ]-------+
```

### Direct Communication Rule
Subagents **do not communicate directly with one another**.
- `optimizer` does not instruct `coder` directly; it delivers the ТЗ to `orchestrator`.
- `coder` does not debate `code-reviewer`; it returns diff and smoke test logs to `orchestrator`.
- `orchestrator` strips all optimizer rationale before invoking `code-reviewer`.

---

## 2. Reviewer Priority & Conflict Resolution

In competitive AI, there is a recurring failure mode: an optimization produces dramatic training speedups or inflated cross-validation scores because it inadvertently introduces **data leakage** or simplifies the problem invalidly.

To prevent this:
1. `code-reviewer` evaluates the code without seeing the Optimizer's sales pitch.
2. If `code-reviewer` returns `REJECT` (due to leakage, validation corruption, or severe architectural fragility), the change is **BLOCKED IMMEDIATELY**.
3. The Optimizer's `APPROVE` cannot override the Code-Reviewer's `REJECT`.
4. The Orchestrator initiates a rollback or instructs the Coder to eliminate the defect.

---

## 3. Experiment Tracking Protocol

Every iteration produces a record in:
- `experiments/metrics.csv`
- `experiments/log.md`

Fields tracked:
- `iteration`: sequential integer index
- `task_id`: problem identifier (e.g. `2_task`, `5_task`)
- `timestamp`: UTC ISO timestamp
- `git_commit`: git commit hash or branch tag
- `hypothesis_id`: ID from the Causal Hypothesis Card
- `cv_metric`: quantitative validation score
- `metric_name`: e.g. minADE, F1, Loss, RuntimeMS
- `train_time_sec`: wall-clock time
- `ram_peak_gb`: peak host memory
- `vram_peak_gb`: peak accelerator memory
- `optimizer_verdict`: APPROVE / REJECT
- `reviewer_verdict`: APPROVE / REJECT
- `status`: COMMITTED / REVERTED / BENCHMARK

---

## 4. Task Directory Isolation Rule (`<N>_task/`)

Each contest problem is strictly contained within its own dedicated directory `<N>_task/`:
- `2_task/` (e.g. C++ algorithmic optimization `best_ver.cpp`)
- `5_task/` (e.g. Trajectory prediction with PointNet, datasets `train/`, `val/`, `test/`, `predictions.bin`)

**Subagent Guidelines**:
1. All changes, builds, models, datasets, and output predictions must be isolated inside `<N>_task/`.
2. Do not write contest code or data directly into the repository root.
3. Submissions must be validated using files inside the specific `<N>_task/` folder.

---

## 4. Pre-Submission Gate

Before generating the final submission file for the competitive platform:
1. Run `python3 .agents/skills/submission-validator/scripts/validate_submission.py --file <path>`.
2. Ensure exit code is 0 (all shape, NaN, and distribution checks pass).
3. Record the validated submission filename in `experiments/log.md`.
