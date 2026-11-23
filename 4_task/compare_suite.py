import sys
import time
from arena import generate_test, parse_input_and_evaluate, run_solver

num_tests = 6
seeds = [101, 202, 303, 404, 505, 606]

total_old = 0.0
total_new = 0.0

print(f"Running comparison suite on {num_tests} tests...")
for idx, s in enumerate(seeds):
    td = generate_test(seed=s)
    
    a_old, d_old = run_solver(['4_task/best_ver'], td)
    sc_old = parse_input_and_evaluate(td, a_old) * 10.0
    
    a_new, d_new = run_solver(['4_task/improved_ver'], td)
    sc_new = parse_input_and_evaluate(td, a_new) * 10.0
    
    diff = sc_new - sc_old
    total_old += sc_old
    total_new += sc_new
    print(f"Test {idx+1}: Old={sc_old:.4f} ({d_old:.2f}s) | New={sc_new:.4f} ({d_new:.2f}s) | Delta={diff:+.4f}")

avg_old = total_old / num_tests
avg_new = total_new / num_tests
print(f"\nAverage Old: {avg_old:.4f} / 10 | Average New: {avg_new:.4f} / 10 | Overall Delta: {avg_new - avg_old:+.4f}")
