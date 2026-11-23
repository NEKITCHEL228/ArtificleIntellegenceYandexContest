#!/usr/bin/env python3
import subprocess
import time
import random
import sys
from pathlib import Path

WORKSPACE = Path(__file__).resolve().parent

def compile_binaries():
    print("Compiling best_ver.cpp and best_ver_opt.cpp with g++ -O3 -std=c++20...")
    cmd1 = ["g++", "-O3", "-std=c++20", str(WORKSPACE / "best_ver.cpp"), "-o", str(WORKSPACE / "best_ver_orig")]
    cmd2 = ["g++", "-O3", "-std=c++20", str(WORKSPACE / "best_ver_opt.cpp"), "-o", str(WORKSPACE / "best_ver_opt")]
    
    subprocess.run(cmd1, check=True)
    subprocess.run(cmd2, check=True)
    print("Compilation successful.")

def test_examples():
    print("\n--- Testing Example 1 ---")
    ex1_in = "4 3\n....\n.##.\n....\n0 0 3 2\n1 0 1 2\n3 2 3 2\n"
    res_orig = subprocess.run([str(WORKSPACE / "best_ver_orig")], input=ex1_in, text=True, capture_output=True).stdout.strip().split()
    res_opt = subprocess.run([str(WORKSPACE / "best_ver_opt")], input=ex1_in, text=True, capture_output=True).stdout.strip().split()
    print("Expected: ['5', '4', '0']")
    print("Orig:    ", res_orig)
    print("Opt:     ", res_opt)
    assert res_opt == ['5', '4', '0'], "Example 1 failed!"
    assert res_orig == res_opt, "Mismatch on Example 1!"

    print("\n--- Testing Example 2 ---")
    ex2_in = "5 3\n..#..\n..#..\n..#..\n0 0 4 2\n0 0 1 2\n3 1 3 1\n"
    res_orig = subprocess.run([str(WORKSPACE / "best_ver_orig")], input=ex2_in, text=True, capture_output=True).stdout.strip().split()
    res_opt = subprocess.run([str(WORKSPACE / "best_ver_opt")], input=ex2_in, text=True, capture_output=True).stdout.strip().split()
    print("Expected: ['-1', '3', '0']")
    print("Orig:    ", res_orig)
    print("Opt:     ", res_opt)
    assert res_opt == ['-1', '3', '0'], "Example 2 failed!"
    assert res_orig == res_opt, "Mismatch on Example 2!"
    print("Both examples verified 100% correct.")

def run_performance_benchmark():
    print("\n--- Running Performance Benchmark (1000x1000 Grid, 5000 Queries) ---")
    W, H = 1000, 1000
    random.seed(42)
    grid = []
    free_cells = []
    
    for y in range(H):
        row = []
        for x in range(W):
            if random.random() < 0.20 and not (x == 0 and y == 0):
                row.append('#')
            else:
                row.append('.')
                free_cells.append((x, y))
        grid.append("".join(row))
        
    num_queries = 5000
    queries = []
    for _ in range(num_queries):
        s = random.choice(free_cells)
        t = random.choice(free_cells)
        queries.append(f"{s[0]} {s[1]} {t[0]} {t[1]}")
        
    input_data = f"{W} {H}\n" + "\n".join(grid) + "\n" + "\n".join(queries) + "\n"
    print(f"Generated map {W}x{H} with {len(queries)} queries ({round(len(input_data)/(1024*1024), 2)} MB input).")
    
    # Run Baseline
    t0 = time.perf_counter()
    p_orig = subprocess.run([str(WORKSPACE / "best_ver_orig")], input=input_data, text=True, capture_output=True)
    orig_time = time.perf_counter() - t0
    out_orig = p_orig.stdout.strip().split()
    
    # Run Optimized
    t0 = time.perf_counter()
    p_opt = subprocess.run([str(WORKSPACE / "best_ver_opt")], input=input_data, text=True, capture_output=True)
    opt_time = time.perf_counter() - t0
    out_opt = p_opt.stdout.strip().split()
    
    # Verification
    assert len(out_orig) == num_queries, f"Orig produced {len(out_orig)} outputs instead of {num_queries}"
    assert len(out_opt) == num_queries, f"Opt produced {len(out_opt)} outputs instead of {num_queries}"
    
    mismatches = 0
    for i in range(num_queries):
        if out_orig[i] != out_opt[i]:
            mismatches += 1
            if mismatches <= 5:
                print(f"Mismatch at query {i} ({queries[i]}): Orig={out_orig[i]}, Opt={out_opt[i]}")
                
    assert mismatches == 0, f"Found {mismatches} mismatches between orig and opt!"
    print("Verification: 100% Exact Match on all 5000 queries!")
    
    # Results
    orig_qps = num_queries / orig_time
    opt_qps = num_queries / opt_time
    speedup = orig_time / opt_time
    
    print("\n================ BENCHMARK RESULTS ================")
    print(f"Baseline (best_ver.cpp):     {orig_time:.3f} sec ({orig_qps:.1f} queries/sec)")
    print(f"Optimized (best_ver_opt.cpp): {opt_time:.3f} sec ({opt_qps:.1f} queries/sec)")
    print(f"SPEEDUP:                      {speedup:.2f}x FASTER!")
    print("===================================================")
    return speedup, orig_qps, opt_qps

if __name__ == "__main__":
    compile_binaries()
    test_examples()
    run_performance_benchmark()
