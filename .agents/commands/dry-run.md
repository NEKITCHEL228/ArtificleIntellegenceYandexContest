---
description: Run an end-to-end dry-run verification cycle of the AI competition multi-agent system on a mock optimization task.
argument-hint: "[optional task name or blank]"
---

# /dry-run

Executes a complete simulated iteration cycle to verify the integrity, communication protocols, context isolation, and consensus mechanisms of the multi-agent system.

## Command Execution

Run the dry-run test suite:

```bash
python3 tests/test_multiagent_system.py
```

## What it Verifies

1. **Causal Reasoning & User Idea Guardrail**:
   - Ensures the hypothesis strictly follows `Observation -> Cause -> Intervention -> Metric -> Criterion`.
   - Confirms system does not generate unprompted ideas.
2. **Five-Agent Council**:
   - Gathers votes from ML Theorist, Pragmatist, Hardware Eng, Leakage Auditor, and ROI Critic.
   - Asserts consensus and absence of auditor veto.
3. **Optimizer ТЗ Generation**:
   - Reads `.hardware_profile.md`.
   - Produces detailed instruction without writing production code.
4. **Coder Implementation & Smoke Test**:
   - Translates spec to code, runs test on minimal slice.
5. **Context-Isolated Code Review**:
   - Verifies Code-Reviewer sees ONLY task, baseline, and diff (NO Optimizer arguments).
   - Validates Reviewer veto authority over Optimizer.
6. **Experiment Logging & Submission Validation**:
   - Logs experiment in `experiments/metrics.csv` and `experiments/log.md`.
   - Runs `.agents/skills/submission-validator/scripts/validate_submission.py`.
