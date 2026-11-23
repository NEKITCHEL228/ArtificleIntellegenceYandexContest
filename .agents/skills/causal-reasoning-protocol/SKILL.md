---
name: causal-reasoning-protocol
description: Formulates, verifies, and audits causal hypothesis cards for every code and model modification. Enforces the strict rule that novel hypotheses are generated ONLY upon explicit user authorization.
metadata:
  origin: ECC
---

# Causal Reasoning Protocol

This skill enforces discipline, falsifiability, and adherence to user-directed development.

## The Causal Hypothesis Card

Every proposed code or configuration change must be documented using this standard template:

```markdown
### Causal Hypothesis Card: [ID - Title]
1. **Observation / Error**:
   - What specific measurement, bottleneck, or failure was observed in the baseline or latest run?
2. **Root Cause Analysis**:
   - Why does this bottleneck exist? (e.g. "Trajectory predictions in Frenet frame suffer from coordinate transform discontinuities at high curvature points").
3. **Proposed Intervention**:
   - Exactly what algorithmic, architectural, or code modification is being introduced?
4. **Expected Impact on Target Metric**:
   - Direction, magnitude, and mechanism (e.g. "Expected to reduce minADE_k by 0.08 on turning maneuvers").
5. **Falsification & Stopping Criterion**:
   - If after 5 epochs / full CV run the metric improves by less than 0.01, or runtime increases by >50%, immediately abandon and rollback.
```

---

## Idea Generation Check

Before generating any new hypothesis, verify:

```
Did the user explicitly request new ideas?
- YES -> Propose up to 3 prioritized causal hypothesis cards for the user to select.
- NO  -> FORBIDDEN to invent new ideas. Analyze, formalize, and execute ONLY the user's idea.
```
