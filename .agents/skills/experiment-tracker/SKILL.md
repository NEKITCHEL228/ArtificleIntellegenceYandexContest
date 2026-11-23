---
name: experiment-tracker
description: Standardized tracking of AI competition experiments, model iterations, CV metrics, resource usage, and reviewer verdicts in experiments/log.md and experiments/metrics.csv.
metadata:
  origin: ECC
---

# Experiment Tracker

A disciplined competition team tracks every hypothesis, git commit, metric score, and reviewer decision to avoid regressions and prevent leaderboard overfitting.

## Storage Locations

1. `experiments/metrics.csv` — Machine-readable historical ledger:
   `iteration,timestamp,git_commit,hypothesis_id,cv_metric,metric_name,train_time_sec,ram_peak_gb,vram_peak_gb,optimizer_verdict,reviewer_verdict,status`

2. `experiments/log.md` — Human-readable experiment logs containing the Causal Hypothesis Card, diff summary, review summaries, and outcome.

## Logging Workflow

1. **Before Running**: Orchestrator assigns an iteration ID (e.g. `exp-001`) and records the hypothesis card.
2. **After Run**: Record validation metric (e.g. Cross-Validation minADE, F1, RMSE).
3. **Review Gate**: Record verdicts from `optimizer` and `code-reviewer`.
4. **Final Status**:
   - `COMMITTED`: Approved and merged to main.
   - `REVERTED`: Rejected by reviewer or failed metric threshold; branch deleted/reverted.
   - `BENCHMARK`: Baseline score.
