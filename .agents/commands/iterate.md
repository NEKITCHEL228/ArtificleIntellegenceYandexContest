---
description: Run a full competition optimization iteration cycle on the current task based on the user's idea.
argument-hint: "<user idea or hypothesis description>"
---

# /iterate

Executes a live, disciplined optimization loop on the current task.

**Input**: `$ARGUMENTS`

---

## Phase 1: Formulation & Council
1. Verify user provided an idea (or explicitly requested hypothesis generation).
2. Formulate Causal Hypothesis Card:
   - Observation / Error
   - Root Cause
   - Proposed Intervention
   - Expected Metric Impact
   - Stopping Criterion
3. Run `five-agent-verdict`:
   ```bash
   python3 .agents/skills/five-agent-verdict/scripts/run_council.py \
     --title "<Title>" --obs "<Observation>" --cause "<Cause>" \
     --intervention "<Intervention>" --metric "<Metric>" --stopping "<Criterion>"
   ```
   If verdict is not `GO`, stop and resolve objections.

## Phase 2: Specification & Implementation
4. `optimizer` reads `.hardware_profile.md` and crafts detailed ТЗ.
5. `coder` implements changes and executes smoke test on small slice.

## Phase 3: Dual Parallel Review & Context Isolation
6. `optimizer` evaluates technical conformance to ТЗ.
7. `code-reviewer` receives **isolated context** (no optimizer rationale!) and evaluates data leakage, numerical stability, and quality.
8. If `code-reviewer` issues `REJECT`, trigger rework or rollback.

## Phase 4: Full Validation & Tracking
9. Run full validation / training run.
10. Record metrics in `experiments/metrics.csv` and `experiments/log.md`.
11. Run `submission-validator`:
    ```bash
    python3 .agents/skills/submission-validator/scripts/validate_submission.py --file <path_to_submission>
    ```
