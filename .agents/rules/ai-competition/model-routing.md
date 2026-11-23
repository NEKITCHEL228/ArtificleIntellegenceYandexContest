# Model Routing Policy & Resource Allocation

## Tier Definitions

| Tier | Capability Profile | Representative Models | Primary Assignment |
|------|--------------------|-----------------------|--------------------|
| **Tier 1 (Top-Tier Reasoning)** | Deep architectural reasoning, mathematical rigor, complex causal deduction, code audit | Claude Opus 4.6, Claude Sonnet 4.6 (extended thinking), Gemini 3.8 Pro | **Orchestrator**, **Optimizer**, **Code-Reviewer**, Council Theorist & Leakage Auditor |
| **Tier 2 (Strong Coding & Execution)** | Fast, clean code synthesis, refactoring, test execution, algorithm translation | Claude Sonnet 4.6, Gemini 3.8 Flash (High), Qwen 2.5 Coder 32B | **Coder**, Council Pragmatist |
| **Tier 3 (Fast / Utility Tier)** | High speed, cost/token efficiency, structured schema extraction, checklist evaluation | Claude Haiku 4.5, Gemini 3.8 Flash-Lite | Council Hardware & ROI Critics, submission validation, log parsing |

---

## Component-to-Model Mapping Matrix

| Component | Default Model Tier | Fallback Tier | Escalation Trigger |
|-----------|--------------------|---------------|-------------------|
| **Orchestrator** | Tier 1 (Reasoning) | Tier 1 (Alternate provider) | Always Tier 1 (requires holistic context management) |
| **Optimizer** | Tier 1 (Reasoning) | Tier 1 (Alternate provider) | Always Tier 1 (requires hardware-aware algorithmic design) |
| **Coder** | Tier 2 (Coding) | Tier 2 | Escalate to **Tier 1** if: (1) 2 consecutive failed smoke tests, (2) CUDA/C++ bindings, (3) complex mathematical algorithm |
| **Code-Reviewer** | Tier 1 (Reasoning) | Tier 1 | Always Tier 1 (must detect subtle data leaks and semantic bugs) |
| **Council: ML Theorist** | Tier 1 (Reasoning) | Tier 2 | High-ambiguity algorithmic decisions |
| **Council: Olympiad Pragmatist** | Tier 2 (Coding/Olympiad) | Tier 3 | Baseline |
| **Council: Hardware Engineer** | Tier 3 (Utility/Fast) | Tier 2 | Deep memory profiling or multi-GPU setup |
| **Council: Leakage Auditor** | Tier 1 (Reasoning) | Tier 1 | Complex temporal or spatial data structures |
| **Council: ROI Critic** | Tier 3 (Utility/Fast) | Tier 2 | Baseline |
| **Submission Validator** | Tier 3 / Scripted | Scripted | Non-LLM deterministic Python script preferred |

---

## Escalation Policy

```
[Coder (Tier 2)] -> Smoke Test Failed?
       |
     (Yes) -> Iteration 1: Retry with Coder (Tier 2)
       |
     (Still Failing) -> Iteration 2 Failed?
       |
     (Yes) -> ESCALATE TO TIER 1 (Top-Tier Reasoning)
              Orchestrator re-routes debugging to Top-Tier Model
```

- When an escalated Coder completes the fix, subsequent routine modifications return to Tier 2.
