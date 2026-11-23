# AI Competition Multi-Agent Orchestration Protocol

## Overview

This protocol governs the autonomous, disciplined multi-agent workflow designed for high-stakes competitive programming in the "Artificial Intelligence" track (ML optimization, algorithm engineering, neural network design/training, validation integrity, and automated testing submissions).

## The Core Lifecycle

Every optimization cycle must strictly follow the sequential, gated pipeline below. No step may be bypassed.

```
+-------------------------------------------------------------------------------+
| 1. User Idea Input (or explicit "generate ideas" prompt from user)            |
+-------------------------------------------------------------------------------+
                                      |
                                      v
+-------------------------------------------------------------------------------+
| 2. Causal Reasoning Card (Observation -> Cause -> Change -> Expected Metric)  |
+-------------------------------------------------------------------------------+
                                      |
                                      v
+-------------------------------------------------------------------------------+
| 3. Five-Agent Council Verdict (GO / REVISE / DROP)                             |
|    - ML Theorist, Pragmatist, Hardware Eng, Leakage Auditor, ROI Critic      |
+-------------------------------------------------------------------------------+
                                      | [Verdict == GO]
                                      v
+-------------------------------------------------------------------------------+
| 4. Hardware Profile Check (.hardware_profile.md)                              |
+-------------------------------------------------------------------------------+
                                      |
                                      v
+-------------------------------------------------------------------------------+
| 5. Optimizer Subagent: Formulate Technical Spec (ТЗ)                           |
|    - Algorithmic, ML & System optimizations tailored to machine limits        |
|    - Does NOT write final code; outputs unambiguous ТЗ for Coder              |
+-------------------------------------------------------------------------------+
                                      |
                                      v
+-------------------------------------------------------------------------------+
| 6. Orchestrator -> Coder Subagent: Implementation & Smoke Test                |
|    - Strict adherence to ECC clean coding standards                           |
|    - Smoke test on data slice; returns clean diff + execution report          |
+-------------------------------------------------------------------------------+
                                      |
                                      v
+-------------------------------------------------------------------------------+
| 7. Parallel Dual Review Gate (Orchestrator enforces context isolation!)        |
|    +-----------------------------+     +----------------------------------+   |
|    | Optimizer Technical Review  |     | Code-Reviewer (ISOLATED CONTEXT) |   |
|    | - Checks spec compliance   |     | - Checks data leakage, bugs,    |   |
|    | - Speed & hardware target   |     |   validation integrity, style    |   |
|    +-----------------------------+     +----------------------------------+   |
|                                                                               |
|    CRITICAL ARBITRATION RULE:                                                 |
|    Code-Reviewer verdict carries HIGHER priority than Optimizer.             |
|    If Code-Reviewer issues REJECT -> BLOCK MERGE (rework or rollback),        |
|    even if Optimizer approved the technical speedup!                          |
+-------------------------------------------------------------------------------+
                                      | [Both APPROVE]
                                      v
+-------------------------------------------------------------------------------+
| 8. Full Validation, Metric Measurement & Experiment Tracking                  |
|    - Record in experiments/log.md and experiments/metrics.csv                 |
+-------------------------------------------------------------------------------+
                                      |
                                      v
+-------------------------------------------------------------------------------+
| 9. Submission Validator                                                       |
|    - Sanity check predictions, schema, NaNs, distributions vs baseline       |
|    - Automated submission to competition platform                             |
+-------------------------------------------------------------------------------+
```

---

## Agent Communication & Isolation Boundaries

1. **Hub-and-Spoke Architecture:**
   - All subagents communicate strictly through the **Orchestrator**.
   - Subagents **never** invoke or message other subagents directly.
2. **Context Isolation for `code-reviewer`:**
   - The Orchestrator passes **ONLY**:
     1. The competition problem statement and metric definition.
     2. The previous stable code version / baseline.
     3. The new git diff / modified files.
   - The Orchestrator **NEVER** passes the Optimizer's rationales, defense briefs, or internal speedup claims to the Code Reviewer.
   - This prevents cognitive anchoring and ensures an uncompromising audit of data leakage, numerical stability, and test validity.
3. **Reviewer Priority Hierarchy:**
   - `Code-Reviewer: REJECT` overrides `Optimizer: APPROVE`.
   - If the code introduces data leakage, validation invalidation, or architectural fragility, it cannot be admitted, no matter how fast it runs.

---

## Delegation Completion Contract

In accordance with ECC standard:
1. **Your final message IS the deliverable.** Never terminate a turn with "waiting for subagents".
2. **Orchestrator owns collection.** All subagent calls are gathered, verified, and synthesized before concluding.
3. **Branch & State Hygiene:** Each experimental hypothesis is implemented on a discrete branch or stash point (`exp/<id>-<name>`). If rejected, it is cleanly rolled back without leaving uncommitted residue.

---

## Task Isolation (`<N>_task/`)

Each contest problem is strictly scoped to its own folder:
- `2_task/`: C++ competitive algorithms, fast I/O, Makefiles, graph algorithms.
- `5_task/`: Trajectory neural networks, PyTorch models, datasets (`train/`, `val/`, `test/`), submissions (`predictions.bin`).
- Any new task MUST be created in a new dedicated directory `<N>_task/`.

Subagents MUST NEVER:
- Place problem-specific datasets or model weights in the repository root.
- Read or modify files from another task folder during a task-specific optimization cycle.
- Mix submissions or metric logs between tasks without tagging `task_id`.
