#!/usr/bin/env python3
import sys
import subprocess
import math
import random
import time

def generate_test(N=None, d=None, T=None, seed=42):
    rng = random.Random(seed)
    if N is None: N = rng.randint(4, 32)
    if d is None: d = rng.randint(2, 6)
    if T is None: T = rng.randint(16, 64)

    alpha_min = 1e-4
    alpha_max = rng.uniform(0.1, 1.0)

    lines = []
    lines.append(f"{N} {d} {T}")
    lines.append(f"{alpha_min:.8f} {alpha_max:.8f}")

    for i in range(N):
        w_i = rng.randint(1, 10)
        lambda_i = rng.uniform(0.01, 1.5)
        M_i = rng.randint(2, 12)
        lines.append(f"{w_i} {lambda_i:.6f} {M_i}")

        s = [rng.uniform(-2.0, 2.0) for _ in range(d)]
        lines.append(" ".join(f"{x:.6f}" for x in s))

        for r in range(M_i):
            a = rng.uniform(0.1, 2.5)
            nu = rng.uniform(0.2, 5.0)
            delta = rng.uniform(0.1, 2.0)
            b = rng.uniform(-2.5, 2.5)
            phi = rng.uniform(-3.14, 3.14)

            # vector v with norm >= 0.1
            while True:
                v = [rng.uniform(-1.0, 1.0) for _ in range(d)]
                norm_v = math.sqrt(sum(x * x for x in v))
                if norm_v >= 0.1:
                    break

            row = [f"{a:.6f}", f"{nu:.6f}", f"{delta:.6f}", f"{b:.6f}", f"{phi:.6f}"] + [f"{x:.6f}" for x in v]
            lines.append(" ".join(row))

    return "\n".join(lines) + "\n"

def parse_input_and_evaluate(input_data, alpha_schedule):
    lines = input_data.strip().splitlines()
    tokens = []
    for line in lines:
        tokens.extend(line.split())

    idx = 0
    N = int(tokens[idx]); idx += 1
    d = int(tokens[idx]); idx += 1
    T = int(tokens[idx]); idx += 1

    alpha_min = float(tokens[idx]); idx += 1
    alpha_max = float(tokens[idx]); idx += 1

    funcs = []
    for i in range(N):
        w_i = int(tokens[idx]); idx += 1
        lambda_i = float(tokens[idx]); idx += 1
        M_i = int(tokens[idx]); idx += 1

        s = [float(tokens[idx + k]) for k in range(d)]
        idx += d

        terms = []
        L_i = 0.0
        for r in range(M_i):
            a = float(tokens[idx]); idx += 1
            nu = float(tokens[idx]); idx += 1
            delta = float(tokens[idx]); idx += 1
            b = float(tokens[idx]); idx += 1
            phi = float(tokens[idx]); idx += 1
            v = [float(tokens[idx + k]) for k in range(d)]
            idx += d
            terms.append((a, nu, delta, b, phi, v))
            L_i -= a

        funcs.append({
            'lambda': lambda_i,
            'M': M_i,
            's': s,
            'terms': terms,
            'L': L_i
        })

    def eval_fn(fn, x):
        f = 0.5 * fn['lambda'] * sum(xi * xi for xi in x)
        g = [fn['lambda'] * xi for xi in x]
        for a, nu, delta, b, phi, v in fn['terms']:
            z = -b + sum(vk * xk for vk, xk in zip(v, x))
            h = delta + z * z
            theta = nu / h + phi
            f += a * math.sin(theta)
            coeff = (2.0 * a * nu * z * math.cos(theta)) / (h * h)
            for k in range(d):
                g[k] -= coeff * v[k]
        return f, g

    q_scores = []
    for fn in funcs:
        x = list(fn['s'])
        f0, g = eval_fn(fn, x)
        D_i = f0 - fn['L']
        best_f = f0
        alive = True

        for t in range(T):
            alpha = max(alpha_min, min(alpha_max, alpha_schedule[t]))
            # gradient step
            nxt_x = [x[k] - alpha * g[k] for k in range(d)]
            # check domain [-10, 10]
            if not all(-10.0 <= xi <= 10.0 and math.isfinite(xi) for xi in nxt_x):
                alive = False
                break
            x = nxt_x
            f_val, g = eval_fn(fn, x)
            if f_val < best_f:
                best_f = f_val

        qi = (f0 - best_f) / D_i if D_i > 0 else 0.0
        qi = max(0.0, min(1.0, qi))
        q_scores.append(qi)

    return sum(q_scores) / N

def run_solver(cmd, input_data):
    t0 = time.perf_counter()
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    stdout, stderr = proc.communicate(input=input_data)
    dur = time.perf_counter() - t0

    if proc.returncode != 0:
        raise RuntimeError(f"Solver failed with code {proc.returncode}: {stderr}")

    alpha_vals = list(map(float, stdout.strip().split()))
    return alpha_vals, dur

def evaluate_benchmark(cmd, num_tests=10, base_seed=1000):
    total_score = 0.0
    total_time = 0.0

    print(f"Running Problem 4 Arena Benchmark on {cmd} ({num_tests} tests)...")

    for idx in range(num_tests):
        test_data = generate_test(seed=base_seed + idx * 79)
        alpha, dur = run_solver(cmd, test_data)
        q = parse_input_and_evaluate(test_data, alpha)
        score_10 = q * 10.0
        total_score += score_10
        total_time += dur
        print(f"  Test {idx+1:2d}: score = {score_10:.4f} / 10.0 (time = {dur:.2f}s, steps = {len(alpha)})")

    avg_score = total_score / num_tests
    print(f"\nFinal Arena Score: {avg_score:.4f} / 10.0 | Total time: {total_time:.2f}s | Avg time per test: {total_time/num_tests:.2f}s")
    return avg_score

if __name__ == '__main__':
    target = sys.argv[1] if len(sys.argv) > 1 else '4_task/best_ver'
    num_tests = int(sys.argv[2]) if len(sys.argv) > 2 else 5
    evaluate_benchmark([target], num_tests=num_tests)
