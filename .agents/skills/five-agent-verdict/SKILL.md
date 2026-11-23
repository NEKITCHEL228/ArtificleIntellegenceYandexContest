---
name: five-agent-verdict
description: Convene a 5-agent specialized jury to evaluate an AI competition idea or hypothesis before coding. Synthesizes independent verdicts from ML Theorist, Olympiad Pragmatist, Systems Engineer, Leakage Auditor, and ROI Critic into a GO / REVISE / DROP consensus.
metadata:
  origin: ECC
---

# Five-Agent Verdict (Competition Idea Council)

Before modifying code or launching compute-intensive experiments, convene this 5-voice independent council to rigorously evaluate the hypothesis.

## The Five Council Roles

| Voice | Tier | Lens & Responsibility |
|-------|------|------------------------|
| **1. ML/Algorithm Theorist** | Tier 1 (Reasoning) | Mathematical validity, inductive bias, loss function convergence guarantees, algorithmic soundness |
| **2. Olympiad Pragmatist** | Tier 2 (Coding/Olympiad) | Realistic competition metric gain, leaderboard sensitivity, baseline headroom, competitive track record |
| **3. Systems & Hardware Engineer** | Tier 3 (Utility/Fast) | Hardware profile compatibility (`.hardware_profile.md`), VRAM/RAM limits, training time feasibility, throughput |
| **4. Skeptic / Leakage Auditor** | Tier 1 (Reasoning) | Data leakage vectors, CV-LB divergence risks, temporal/spatial group contamination, fragile assumptions |
| **5. Complexity & ROI Critic** | Tier 3 (Utility/Fast) | Implementation complexity, debugging surface, lines of code vs expected return on investment |

---

## Anti-Anchoring Isolation

Each subagent evaluates the proposal independently with **ONLY**:
1. The formal task formulation and evaluation metric.
2. The current baseline score and model architecture.
3. The Causal Hypothesis Card.

Subagents do **NOT** see each other's opinions until their individual evaluations are logged.

---

## Council Output Format

```markdown
## Five-Agent Council: [Hypothesis Title]

1. **ML/Algorithm Theorist**: [GO / REVISE / DROP]
   - *Rationale*: ...
2. **Olympiad Pragmatist**: [GO / REVISE / DROP]
   - *Rationale*: ...
3. **Systems & Hardware Engineer**: [GO / REVISE / DROP]
   - *Rationale*: ...
4. **Skeptic / Leakage Auditor**: [GO / REVISE / DROP]
   - *Rationale*: ...
5. **Complexity & ROI Critic**: [GO / REVISE / DROP]
   - *Rationale*: ...

---
### Synthesized Verdict: [GO / REVISE / DROP]
- **Consensus Score**: [X/5 Votes for GO]
- **Key Risks & Failure Modes**:
  1. ...
  2. ...
- **Mandatory Directives for Optimizer**:
  - ...
```

### Verdict Rules:
- **`GO`**: Minimum 4/5 positive votes AND 0 unconditional vetos from Leakage Auditor.
- **`REVISE`**: Objections raised regarding hardware feasibility, complexity, or minor assumptions. Returns to user/optimizer for spec refinement.
- **`DROP`**: Mathematical invalidity, fatal data leakage, or negative expected ROI. Rejected without touching code.
