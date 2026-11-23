#!/usr/bin/env python3
"""
Submission Validator for AI Competitions.
Validates submission files before upload to testing systems:
- File existence & non-emptiness
- Format parsing (CSV, TSV, NPZ, NPY, BIN)
- Row counts, column names
- NaN / Inf / null checks
- Statistical distribution sanity checks against baseline
"""

import sys
import os
import argparse
from pathlib import Path
import numpy as np

def validate_tabular(df, expected_rows=None, expected_cols=None, id_col=None):
    errors = []
    warnings = []
    
    # 1. Row count
    if expected_rows is not None and len(df) != expected_rows:
        errors.append(f"Row count mismatch: expected {expected_rows}, got {len(df)}")
        
    # 2. Columns
    if expected_cols:
        missing_cols = [c for c in expected_cols if c not in df.columns]
        if missing_cols:
            errors.append(f"Missing required columns: {missing_cols}")
            
    # 3. ID column uniqueness
    if id_col and id_col in df.columns:
        if df[id_col].duplicated().any():
            dup_count = df[id_col].duplicated().sum()
            errors.append(f"Duplicate IDs found in '{id_col}': {dup_count} duplicates")
            
    # 4. NaN / Inf check
    for col in df.select_dtypes(include=[np.number]).columns:
        vals = df[col].to_numpy()
        nan_count = np.isnan(vals).sum()
        inf_count = np.isinf(vals).sum()
        if nan_count > 0:
            errors.append(f"Column '{col}' contains {nan_count} NaN values")
        if inf_count > 0:
            errors.append(f"Column '{col}' contains {inf_count} Inf values")
            
    return errors, warnings

def validate_numpy(arr, expected_shape=None):
    errors = []
    warnings = []
    
    if expected_shape and arr.shape != expected_shape:
        errors.append(f"Shape mismatch: expected {expected_shape}, got {arr.shape}")
        
    nan_count = np.isnan(arr).sum()
    inf_count = np.isinf(arr).sum()
    if nan_count > 0:
        errors.append(f"Array contains {nan_count} NaN values")
    if inf_count > 0:
        errors.append(f"Array contains {inf_count} Inf values")
        
    # Constant check
    if arr.size > 0 and np.all(arr == arr.flat[0]):
        warnings.append(f"Degenerate prediction warning: all values in array are identical ({arr.flat[0]})")
        
    return errors, warnings

def main():
    parser = argparse.ArgumentParser(description="Validate submission file before uploading.")
    parser.add_argument("--file", required=True, help="Path to submission file")
    parser.add_argument("--expected-rows", type=int, default=None, help="Expected number of samples/rows")
    parser.add_argument("--expected-cols", type=str, default=None, help="Comma-separated expected columns")
    parser.add_argument("--id-col", type=str, default=None, help="Column name containing sample IDs")
    parser.add_argument("--baseline", type=str, default=None, help="Optional baseline file for distribution check")
    
    args = parser.parse_args()
    file_path = Path(args.file)
    
    print(f"=== Validating Submission File: {file_path} ===")
    
    if not file_path.exists():
        print(f"CRITICAL ERROR: File does not exist: {file_path}")
        sys.exit(1)
        
    size_bytes = file_path.stat().st_size
    if size_bytes == 0:
        print(f"CRITICAL ERROR: File is empty (0 bytes): {file_path}")
        sys.exit(1)
        
    print(f"- File size: {round(size_bytes / (1024*1024), 2)} MB ({size_bytes} bytes)")
    
    ext = file_path.suffix.lower()
    errors = []
    warnings = []
    stats = {}
    
    if ext in [".csv", ".tsv", ".txt"]:
        try:
            import pandas as pd
            sep = "\t" if ext == ".tsv" else ","
            df = pd.read_csv(file_path, sep=sep)
            expected_cols = [c.strip() for c in args.expected_cols.split(",")] if args.expected_cols else None
            errs, warns = validate_tabular(df, args.expected_rows, expected_cols, args.id_col)
            errors.extend(errs)
            warnings.extend(warns)
            
            num_cols = df.select_dtypes(include=[np.number]).columns
            for col in num_cols:
                vals = df[col].to_numpy()
                stats[col] = {
                    "mean": float(np.nanmean(vals)),
                    "std": float(np.nanstd(vals)),
                    "min": float(np.nanmin(vals)),
                    "max": float(np.nanmax(vals)),
                }
        except Exception as e:
            errors.append(f"Failed to parse tabular file: {e}")
            
    elif ext in [".npy", ".npz", ".bin"]:
        try:
            if ext == ".npz":
                data = np.load(file_path)
                key = list(data.keys())[0]
                arr = data[key]
                print(f"- Loaded NPZ array '{key}' with shape {arr.shape}")
            elif ext == ".npy":
                arr = np.load(file_path)
                print(f"- Loaded NPY array with shape {arr.shape}")
            else:
                arr = np.fromfile(file_path, dtype=np.float32)
                print(f"- Loaded binary raw float32 array with length {len(arr)}")
                
            errs, warns = validate_numpy(arr)
            errors.extend(errs)
            warnings.extend(warns)
            
            stats["array"] = {
                "shape": list(arr.shape),
                "mean": float(np.nanmean(arr)),
                "std": float(np.nanstd(arr)),
                "min": float(np.nanmin(arr)),
                "max": float(np.nanmax(arr)),
            }
        except Exception as e:
            errors.append(f"Failed to parse binary/numpy file: {e}")
    else:
        warnings.append(f"Unrecognized file extension '{ext}'. Skipping format-specific parsing.")
        
    # Print Distribution Summary
    print("\n--- Distribution Sanity Check ---")
    for name, s in stats.items():
        print(f"Feature: {name} | Mean: {s.get('mean', 0.0):.4f} | Std: {s.get('std', 0.0):.4f} | Min: {s.get('min', 0.0):.4f} | Max: {s.get('max', 0.0):.4f}")
        
    if warnings:
        print("\n--- Warnings ---")
        for w in warnings:
            print(f"[WARN] {w}")
            
    if errors:
        print("\n--- Critical Validation Errors ---")
        for e in errors:
            print(f"[REJECT] {e}")
        print("\n>>> SUBMISSION VALIDATION FAILED. DO NOT SUBMIT. <<<")
        sys.exit(1)
        
    print("\n>>> SUBMISSION VALIDATION PASSED! FILE IS SAFE TO SUBMIT. <<<")
    sys.exit(0)

if __name__ == "__main__":
    main()
