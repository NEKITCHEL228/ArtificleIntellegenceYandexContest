---
name: submission-validator
description: Validates submission prediction files (CSV, TSV, NPZ, NPY, BIN) before uploading to competition testing systems. Checks file integrity, row counts, NaN/Inf values, schema, and statistical distributions.
metadata:
  origin: ECC
---

# Submission Validator

Automated pre-flight check for contest submissions to prevent wasted submission quota, formatting disqualifications, or corrupted predictions.

## Checks Performed

1. **Integrity & Size**: Verifies file exists and has non-zero size.
2. **Schema & Count**: Confirms expected row/sample count and column identifiers match contest test set.
3. **Numerical Safety**: Asserts 0 `NaN`, 0 `Inf`, and no invalid sentinel values.
4. **Distribution Sanity**: Checks mean, standard deviation, and range against degenerate outputs (e.g. constant zero predictions).

## CLI Invocation

```bash
# For tabular CSV submissions:
python3 .agents/skills/submission-validator/scripts/validate_submission.py \
  --file path/to/submission.csv \
  --expected-rows 1000 \
  --expected-cols "id,prediction" \
  --id-col id

# For NumPy / Binary predictions:
python3 .agents/skills/submission-validator/scripts/validate_submission.py \
  --file path/to/predictions.npz
```

Exit code:
- `0`: Valid and safe to submit.
- `1`: Critical validation errors found. Upload is blocked.
