#!/usr/bin/env python3
import sys
import subprocess
import random
import time
from pathlib import Path

# --- BOT IMPLEMENTATIONS FOR ARENA SIMULATION ---

def bot_greedy_v(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid: return 0
    return max(valid, key=lambda i: (v[i], -m[i], -i))

def bot_greedy_ratio(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid: return 0
    return max(valid, key=lambda i: (v[i] / m[i], v[i], -i))

def bot_safe_w1(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid: return 0
    safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i] / m[i], v[i], -i))

def bot_safe_w2(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid: return 0
    safe = [i for i in valid if cur_m + m[i] <= max(1, W - 2)]
    if not safe:
        safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i] / m[i], v[i], -i))

def bot_beat_max(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid: return 0
    limit = max_other if max_other > 0 else W
    safe = [i for i in valid if cur_m + m[i] < limit]
    if not safe:
        safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i], -m[i], -i))

def bot_min_m(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid: return 0
    return min(valid, key=lambda i: (m[i], -v[i], i))

ARENA_BOTS = [
    ('greedy_v', bot_greedy_v),
    ('greedy_ratio', bot_greedy_ratio),
    ('safe_w1', bot_safe_w1),
    ('safe_w2', bot_safe_w2),
    ('beat_max', bot_beat_max),
    ('min_m', bot_min_m),
]

def generate_game(seed=42):
    random.seed(seed)
    K = random.randint(2, 6)
    N = random.randint(15, 60)
    T = random.randint(5, 15)
    W = random.randint(15, 45)

    m = [0] + [random.randint(1, min(W, 30)) for _ in range(N)]
    v = [0] + [random.randint(1, 100) for _ in range(N)]

    bot_name, bot_func = random.choice(ARENA_BOTS)
    return K, N, T, W, m, v, bot_name, bot_func

def solve_centralized_max(N, W, m, v):
    # Standard 0/1 knapsack for theoretical max value in capacity W
    dp = [0] * (W + 1)
    for i in range(1, N + 1):
        wi, vi = m[i], v[i]
        if wi <= W:
            for w in range(W, wi - 1, -1):
                if dp[w - wi] + vi > dp[w]:
                    dp[w] = dp[w - wi] + vi
    return max(dp)

def run_match(cmd, K, N, T, W, m, v, bot_func):
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1
    )

    # Initial input
    proc.stdin.write(f"{K} {N} {T} {W}\n")
    proc.stdin.write(" ".join(map(str, m[1:])) + "\n")
    proc.stdin.write(" ".join(map(str, v[1:])) + "\n")
    proc.stdin.flush()

    avail = set(range(1, N + 1))
    M = [0] * K
    V = [0] * K

    for r in range(1, T + 1):
        # 1. Read player 1 move
        line = proc.stdout.readline()
        if not line:
            proc.kill()
            return False, 0, "Agent terminated early or EOF"
        try:
            a1 = int(line.strip())
        except ValueError:
            proc.kill()
            return False, 0, f"Invalid move output: {line.strip()}"

        # Validate a1
        valid_p1 = [i for i in avail if m[i] <= W - M[0]]
        if a1 == 0:
            if valid_p1:
                proc.kill()
                return False, 0, f"Illegal skip when items fit: {valid_p1[:3]}"
        else:
            if a1 not in avail:
                proc.kill()
                return False, 0, f"Item {a1} already taken or invalid"
            if m[a1] > W - M[0]:
                proc.kill()
                return False, 0, f"Item {a1} exceeds capacity (m={m[a1]} > {W - M[0]})"
            avail.remove(a1)
            M[0] += m[a1]
            V[0] += v[a1]

        # 2. Bots make moves
        bot_moves = []
        for d in range(1, K):
            max_other_d = max((M[j] for j in range(K) if j != d), default=0)
            ad = bot_func(avail, M[d], W, max_other_d, m, v)
            bot_moves.append(ad)
            if ad > 0:
                avail.remove(ad)
                M[d] += m[ad]
                V[d] += v[ad]

        # 3. Send bot moves to player
        proc.stdin.write(" ".join(map(str, bot_moves)) + "\n")
        proc.stdin.flush()

    proc.stdin.close()
    proc.stdout.close()
    proc.stderr.close()
    proc.wait()

    # Dragon evaluation
    max_m = max(M)
    if M[0] < max_m:
        survived = True
        score = V[0]
    else:
        survived = False
        score = 0
    return survived, score, f"M0={M[0]} M_max={max_m} V0={V[0]}"

def evaluate_agent(cmd, num_games=40, seed=123):
    total_q = 0.0
    total_max = 0.0
    survivals = 0

    print(f"Running Arena benchmark ({num_games} games) on {cmd}...")
    t0 = time.perf_counter()

    for g_idx in range(num_games):
        K, N, T, W, m, v, bot_name, bot_func = generate_game(seed=seed + g_idx * 17)
        v_max = solve_centralized_max(N, W, m, v)
        if v_max == 0: v_max = 1

        survived, score, detail = run_match(cmd, K, N, T, W, m, v, bot_func)
        if survived:
            survivals += 1
            q_ratio = score / v_max
        else:
            q_ratio = 0.0

        total_q += q_ratio
        total_max += 1.0

    dur = time.perf_counter() - t0
    final_score = (total_q / total_max) * 10.0
    surv_rate = (survivals / num_games) * 100.0

    print(f"Arena Score: {final_score:.3f} / 10.0  | Survival Rate: {surv_rate:.1f}% ({survivals}/{num_games}) | Time: {dur:.2f}s")
    return final_score, surv_rate

if __name__ == '__main__':
    target = sys.argv[1] if len(sys.argv) > 1 else '1_task/best_ver.py'
    num_games = int(sys.argv[2]) if len(sys.argv) > 2 else 30
    if target.endswith('.py'):
        cmd = ['python3', target]
    else:
        cmd = [target]
    evaluate_agent(cmd, num_games=num_games)
