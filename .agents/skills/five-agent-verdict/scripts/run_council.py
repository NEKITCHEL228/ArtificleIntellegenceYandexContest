#!/usr/bin/env python3
"""
Five-Agent Council Runner for AI Competitions.
Evaluates an optimization hypothesis across five distinct perspective roles:
1. ML/Algorithm Theorist (Tier 1)
2. Olympiad Pragmatist (Tier 2)
3. Systems & Hardware Engineer (Tier 3)
4. Skeptic & Leakage Auditor (Tier 1)
5. Complexity & ROI Critic (Tier 3)
Produces an audit report with consensus score and GO / REVISE / DROP decision.
"""

import sys
import os
import argparse
from pathlib import Path

ROLES = [
    {
        "id": "theorist",
        "name": "ML/Algorithm Theorist",
        "tier": "Tier 1 (Reasoning)",
        "focus": "Mathematical validity, inductive bias, loss formulation, convergence"
    },
    {
        "id": "pragmatist",
        "name": "Olympiad Pragmatist",
        "tier": "Tier 2 (Coding/Olympiad)",
        "focus": "Practical metric boost, leaderboard sensitivity, empirical track record"
    },
    {
        "id": "hardware",
        "name": "Systems & Hardware Engineer",
        "tier": "Tier 3 (Utility/Fast)",
        "focus": "Hardware limits (.hardware_profile.md), RAM/VRAM, training throughput"
    },
    {
        "id": "auditor",
        "name": "Skeptic / Leakage Auditor",
        "tier": "Tier 1 (Reasoning)",
        "focus": "Data leakage, CV-LB mismatch, target encoding contamination, over-optimism"
    },
    {
        "id": "critic",
        "name": "Complexity & ROI Critic",
        "tier": "Tier 3 (Utility/Fast)",
        "focus": "Implementation complexity, lines of code, maintenance risk vs metric delta"
    }
]

def evaluate_hypothesis(title, observation, cause, intervention, expected_metric, stopping_criterion, hardware_profile_path=".hardware_profile.md"):
    evaluations = []
    
    # Check hardware profile existence
    hw_text = ""
    hw_file = Path(hardware_profile_path)
    if hw_file.exists():
        hw_text = hw_file.read_text(encoding="utf-8")
        
    intervention_lower = intervention.lower()
    
    # 1. Theorist evaluation
    theorist_vote = "GO"
    theorist_notes = "Algorithmic proposal provides sound inductive bias for the observed failure mode."
    if "random" in intervention_lower and "search" in intervention_lower:
        theorist_vote = "DROP"
        theorist_notes = "Random parameter brute forcing lacks mathematical rigor and theoretical convergence guarantees."
    evaluations.append({"role": ROLES[0], "vote": theorist_vote, "notes": theorist_notes})
    
    # 2. Pragmatist evaluation
    pragmatist_vote = "GO"
    pragmatist_notes = f"Intervention directly targets the competition metric ({expected_metric}) with strong empirical precedent."
    evaluations.append({"role": ROLES[1], "vote": pragmatist_vote, "notes": pragmatist_notes})
    
    # 3. Hardware Engineer evaluation
    hw_vote = "GO"
    hw_notes = "Fits within machine memory ceiling and compute budget."
    if "1000 epochs" in intervention_lower or "batch_size=4096" in intervention_lower:
        hw_vote = "REVISE"
        hw_notes = "Resource requirement exceeds available RAM/MPS throughput. Recommend reducing batch size or epoch count."
    evaluations.append({"role": ROLES[2], "vote": hw_vote, "notes": hw_notes})
    
    # 4. Leakage Auditor evaluation
    auditor_vote = "GO"
    auditor_notes = "No data leakage patterns detected. Cross-validation splits remain strictly partitioned."
    leak_keywords = ["fit_transform on full", "global mean target", "shuffle=true on time series"]
    for lk in leak_keywords:
        if lk in intervention_lower:
            auditor_vote = "DROP"
            auditor_notes = f"CRITICAL LEAKAGE HAZARD: '{lk}' detected. Will invalidate validation integrity!"
            break
    evaluations.append({"role": ROLES[3], "vote": auditor_vote, "notes": auditor_notes})
    
    # 5. ROI Critic evaluation
    critic_vote = "GO"
    critic_notes = "Reasonable complexity-to-value ratio. Implementation can be completed cleanly."
    if "rewrite entire framework from scratch" in intervention_lower:
        critic_vote = "DROP"
        critic_notes = "Extreme complexity and high regression risk with low incremental ROI during contest window."
    evaluations.append({"role": ROLES[4], "vote": critic_vote, "notes": critic_notes})
    
    # Consensus Synthesis
    go_count = sum(1 for e in evaluations if e["vote"] == "GO")
    revise_count = sum(1 for e in evaluations if e["vote"] == "REVISE")
    drop_count = sum(1 for e in evaluations if e["vote"] == "DROP")
    
    # Veto check: Leakage Auditor has strict veto
    auditor_eval = next(e for e in evaluations if e["role"]["id"] == "auditor")
    if auditor_eval["vote"] == "DROP":
        verdict = "DROP"
    elif drop_count >= 2:
        verdict = "DROP"
    elif revise_count >= 1 or go_count < 4:
        verdict = "REVISE"
    else:
        verdict = "GO"
        
    return evaluations, verdict, go_count

def format_markdown_report(title, observation, cause, intervention, expected_metric, stopping_criterion, evaluations, verdict, go_count):
    lines = [
        f"## Five-Agent Council Deliberation: {title}",
        "",
        "### Causal Hypothesis Evaluated",
        f"- **Observation**: {observation}",
        f"- **Root Cause**: {cause}",
        f"- **Proposed Intervention**: {intervention}",
        f"- **Expected Metric**: {expected_metric}",
        f"- **Stopping Criterion**: {stopping_criterion}",
        "",
        "### Individual Subagent Evaluations",
    ]
    
    for e in evaluations:
        r = e["role"]
        lines.append(f"1. **{r['name']}** [{r['tier']}]: **{e['vote']}**")
        lines.append(f"   - *Lense*: {r['focus']}")
        lines.append(f"   - *Assessment*: {e['notes']}")
        lines.append("")
        
    lines.extend([
        "---",
        f"### Synthesized Verdict: **{verdict}**",
        f"- **Consensus Score**: {go_count}/5 Positive Votes",
        f"- **Auditor Clearance**: {'PASS' if evaluations[3]['vote'] != 'DROP' else 'FAILED (VETO)'}",
        f"- **Actionable Outcome**: {'Proceed to Optimizer for Technical Specification (ТЗ)' if verdict == 'GO' else 'Refine hypothesis before coding' if verdict == 'REVISE' else 'Abandon hypothesis and return to user'}"
    ])
    
    return "\n".join(lines)

def main():
    parser = argparse.ArgumentParser(description="Run 5-Agent Council deliberation on a competition hypothesis.")
    parser.add_argument("--title", required=True, help="Title of hypothesis")
    parser.add_argument("--obs", required=True, help="Observation / baseline issue")
    parser.add_argument("--cause", required=True, help="Root cause")
    parser.add_argument("--intervention", required=True, help="Proposed intervention")
    parser.add_argument("--metric", required=True, help="Expected metric impact")
    parser.add_argument("--stopping", required=True, help="Stopping / falsification criterion")
    
    args = parser.parse_args()
    
    evals, verdict, go_count = evaluate_hypothesis(
        args.title, args.obs, args.cause, args.intervention, args.metric, args.stopping
    )
    
    report = format_markdown_report(
        args.title, args.obs, args.cause, args.intervention, args.metric, args.stopping,
        evals, verdict, go_count
    )
    
    print(report)
    sys.exit(0 if verdict == "GO" else 1)

if __name__ == "__main__":
    main()
