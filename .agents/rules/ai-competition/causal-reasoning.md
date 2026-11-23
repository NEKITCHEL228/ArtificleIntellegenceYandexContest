# Causal Reasoning Protocol & Idea Generation Governance

## The Causal Reasoning Principle

In competitive AI programming, stochastic trial-and-error and unprincipled hyperparameter thrashing consume compute and lead to catastrophic overfitting on public validation sets. 

Before ANY line of code is modified, the Orchestrator and Optimizer MUST formulate an explicit, falsifiable Causal Hypothesis Card:

```markdown
### Causal Hypothesis Card
- **Observation / Baseline Issue**: [Concrete metric, loss curve behavior, profiling bottleneck, or error log]
- **Root Cause**: [Theoretical, statistical, or computational root cause explaining the observation]
- **Proposed Intervention**: [Exact architectural change, algorithmic optimization, or data pipeline modification]
- **Expected Impact on Target Metric**: [Expected metric shift, e.g. ADE/FDE decrease by > 0.05, throughput increase by 2x]
- **Falsification & Stopping Criterion**: [Under what exact measurement is this hypothesis considered failed and rolled back?]
```

---

## Strict Rule: Idea Generation Boundary

### Default Stance (User-Driven)
By default, the multi-agent system operates **STRICTLY ON IDEAS PROVIDED BY THE USER**.
- The Orchestrator does NOT invent new strategic directions unprompted.
- The Orchestrator decomposes, formalizes, subjects to council review, specifies, and implements the user's idea.

### Explicit User Authorization Exception
The system is permitted to brainstorm and propose novel ideas **ONLY IF THE USER EXPLICITLY ASKS FOR THEM**, using phrases such as:
- «придумай идеи» / "brainstorm ideas"
- «предложи гипотезы» / "propose hypotheses"
- «что еще можно попробовать?» / "what else should we try?"
- «проанализируй и предложи варианты» / "analyze and suggest options"

When this exception is triggered:
1. Provide a curated list of maximum 3 prioritized causal hypotheses.
2. Wait for the user to select which hypothesis to pursue.
3. Once selected, return immediately to the default execution protocol.

---

## Anti-Thrashing Guardrails

1. **No Blind Grid Searches**: Tuning parameters without a diagnostic curve (e.g. learning rate range test, gradient norm check) is forbidden.
2. **Leakage Pre-Check**: Any feature engineered from targets or global statistics must be verified across cross-validation folds before inclusion.
3. **One Intervention at a Time**: Never conflate model architecture changes, loss weighting, and feature additions in a single commit unless mandated by the core concept.
