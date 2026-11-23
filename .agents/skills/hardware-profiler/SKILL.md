---
name: hardware-profiler
description: Inspects current machine capabilities (CPU, RAM, GPU/MPS acceleration, disk space, and installed ML libraries) and records them into .hardware_profile.md for the optimizer subagent.
metadata:
  origin: ECC
---

# Hardware Profiler

Produces an actionable `.hardware_profile.md` containing the physical constraints of the execution machine.

## When to Run

- At the beginning of a competition session.
- Whenever hardware environment or Python dependencies change.
- Before `optimizer` formulates a batch size, parallelism, or model scaling plan.

## Invocation

```bash
python3 .agents/skills/hardware-profiler/scripts/profile_hardware.py
```

The output file `.hardware_profile.md` is committed to the workspace root and consumed directly by `agents/optimizer.md`.
