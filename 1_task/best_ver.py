import sys
import time


# Быстрое и корректное решение задачи о рюкзаке (битовые маски исключают дубли)
def solve_knapsack(items, cap, m, v):
    if cap <= 0 or not items:
        return 0, []
    valid = [it for it in items if m[it] <= cap]
    if not valid:
        return 0, []
    if sum(m[it] for it in valid) <= cap:
        return sum(v[it] for it in valid), valid

    dp = [0] * (cap + 1)
    mask = [0] * (cap + 1)
    for it in valid:
        wi, vi = m[it], v[it]
        bit = 1 << it
        for w in range(cap, wi - 1, -1):
            nv = dp[w - wi] + vi
            if nv > dp[w]:
                dp[w] = nv
                mask[w] = mask[w - wi] | bit

    best_w = 0
    max_v = 0
    for w in range(1, cap + 1):
        if dp[w] > max_v:
            max_v = dp[w]
            best_w = w

    chosen_mask = mask[best_w]
    chosen = [it for it in valid if (chosen_mask >> it) & 1]
    return max_v, chosen


# Библиотека возможных стратегий ботов жюри
def bot_greedy_v_minm(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    return max(valid, key=lambda i: (v[i], -m[i], -i))


def bot_greedy_v_maxm(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    return max(valid, key=lambda i: (v[i], m[i], -i))


def bot_greedy_ratio_maxv(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    return max(valid, key=lambda i: (v[i] / m[i], v[i], -i))


def bot_greedy_ratio_minm(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    return max(valid, key=lambda i: (v[i] / m[i], -m[i], -i))


def bot_safe_greedy_v(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i], -m[i], -i))


def bot_safe_greedy_ratio(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i] / m[i], v[i], -i))


def bot_beat_max_v(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    limit = max_other if max_other > 0 else W
    safe = [i for i in valid if cur_m + m[i] < limit]
    if not safe:
        safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i], -m[i], -i))


def bot_beat_max_ratio(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    limit = max_other if max_other > 0 else W
    safe = [i for i in valid if cur_m + m[i] < limit]
    if not safe:
        safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i] / m[i], v[i], -i))


def bot_knapsack_v(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    _, best_items = solve_knapsack(valid, W - cur_m, m, v)
    if best_items:
        return max(best_items, key=lambda i: (v[i], -m[i], -i))
    return max(valid, key=lambda i: (v[i], -m[i], -i))


def bot_safe_knapsack_v(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    target_cap = W - 1 - cur_m
    if target_cap > 0:
        _, best_items = solve_knapsack(valid, target_cap, m, v)
        if best_items:
            return max(best_items, key=lambda i: (v[i], -m[i], -i))
    safe = [i for i in valid if cur_m + m[i] < W]
    pool = safe if safe else valid
    return max(pool, key=lambda i: (v[i], -m[i], -i))


def bot_greedy_min_m(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    return min(valid, key=lambda i: (m[i], -v[i], i))


def bot_greedy_max_m(avail, cur_m, W, max_other, m, v):
    valid = [i for i in avail if m[i] <= W - cur_m]
    if not valid:
        return 0
    return max(valid, key=lambda i: (m[i], v[i], -i))


BOT_CANDIDATES = [
    ('greedy_v_minm', bot_greedy_v_minm),
    ('greedy_v_maxm', bot_greedy_v_maxm),
    ('greedy_ratio_maxv', bot_greedy_ratio_maxv),
    ('greedy_ratio_minm', bot_greedy_ratio_minm),
    ('safe_greedy_v', bot_safe_greedy_v),
    ('safe_greedy_ratio', bot_safe_greedy_ratio),
    ('beat_max_v', bot_beat_max_v),
    ('beat_max_ratio', bot_beat_max_ratio),
    ('knapsack_v', bot_knapsack_v),
    ('safe_knapsack_v', bot_safe_knapsack_v),
    ('greedy_min_m', bot_greedy_min_m),
    ('greedy_max_m', bot_greedy_max_m),
]


def rollout_game(start_cand, cur_r, T, K, W, m, v, init_avail, init_M, init_V, bot_strat_func, p1_policy_name):
    avail = set(init_avail)
    M = list(init_M)
    V = list(init_V)

    # Ход P1 в текущем раунде
    if start_cand > 0:
        avail.remove(start_cand)
        M[0] += m[start_cand]
        V[0] += v[start_cand]

    # Доигрывание текущего раунда ботами
    for d in range(1, K):
        max_other_d = max((M[j] for j in range(K) if j != d), default=0)
        ad = bot_strat_func(avail, M[d], W, max_other_d, m, v)
        if ad > 0:
            avail.remove(ad)
            M[d] += m[ad]
            V[d] += v[ad]

    target_W = W - 1
    if p1_policy_name == 'target_w_minus_2':
        target_W = max(1, W - 2)
    elif p1_policy_name == 'target_w_minus_3':
        target_W = max(1, W - 3)

    # Будущие раунды
    for _ in range(cur_r + 1, T + 1):
        if not avail:
            break
        val_p1 = [i for i in avail if m[i] <= W - M[0]]
        if val_p1:
            if p1_policy_name == 'greedy_v':
                safe = [i for i in val_p1 if M[0] + m[i] < W]
                pool = safe if safe else val_p1
                fut_a = max(pool, key=lambda i: (v[i], -m[i], -i))
            elif p1_policy_name == 'greedy_ratio':
                safe = [i for i in val_p1 if M[0] + m[i] < W]
                pool = safe if safe else val_p1
                fut_a = max(pool, key=lambda i: (v[i] / m[i], v[i], -i))
            elif p1_policy_name in ('target_w_minus_1', 'target_w_minus_2', 'target_w_minus_3'):
                safe = [i for i in val_p1 if M[0] + m[i] <= target_W]
                if safe:
                    fut_a = max(safe, key=lambda i: (v[i] / m[i], v[i], -i))
                else:
                    safe_under_w = [i for i in val_p1 if M[0] + m[i] < W]
                    pool = safe_under_w if safe_under_w else val_p1
                    fut_a = max(pool, key=lambda i: (v[i] / m[i], v[i], -i))
            elif p1_policy_name == 'stay_light':
                max_bots = max(M[1:]) if K > 1 else W
                safe = [i for i in val_p1 if M[0] + m[i] < max_bots]
                if safe:
                    fut_a = max(safe, key=lambda i: (v[i] / m[i], v[i], -i))
                else:
                    fut_a = min(val_p1, key=lambda i: (m[i], -v[i], i))
            elif p1_policy_name == 'min_m':
                fut_a = min(val_p1, key=lambda i: (m[i], -v[i], i))
            else:
                fut_a = val_p1[0]

            avail.remove(fut_a)
            M[0] += m[fut_a]
            V[0] += v[fut_a]

        for d in range(1, K):
            max_other_d = max((M[j] for j in range(K) if j != d), default=0)
            ad = bot_strat_func(avail, M[d], W, max_other_d, m, v)
            if ad > 0:
                avail.remove(ad)
                M[d] += m[ad]
                V[d] += v[ad]

    max_other = max(M[1:]) if K > 1 else 0
    survived = M[0] < max_other
    margin = max_other - M[0]
    return survived, (V[0] if survived else 0), margin


class DwarfAgent:
    def __init__(self, K, N, T, W, m, v, start_time):
        self.K = K
        self.N = N
        self.T = T
        self.W = W
        self.m = m
        self.v = v
        self.start_time = start_time

        self.bot_weights = {name: 1.0 for name, _ in BOT_CANDIDATES}
        self.matches = {name: 0 for name, _ in BOT_CANDIDATES}
        self.total_obs = 0

        # Инкрементальное состояние для быстрой проверки стратегий ботов
        self.sim_avail = set(range(1, N + 1))
        self.sim_M = [0] * K

    def observe_round(self, r_moves):
        p1_m = r_moves[0]
        if p1_m > 0 and p1_m in self.sim_avail:
            self.sim_avail.remove(p1_m)
            self.sim_M[0] += self.m[p1_m]

        for d in range(1, self.K):
            ad = r_moves[d]
            self.total_obs += 1
            max_other_d = max((self.sim_M[j] for j in range(self.K) if j != d), default=0)
            for name, strat_func in BOT_CANDIDATES:
                pred = strat_func(self.sim_avail, self.sim_M[d], self.W, max_other_d, self.m, self.v)
                if pred == ad:
                    self.matches[name] += 1
            if ad > 0:
                if ad in self.sim_avail:
                    self.sim_avail.remove(ad)
                self.sim_M[d] += self.m[ad]

        if self.total_obs > 0:
            max_match = max(self.matches.values())
            for name, _ in BOT_CANDIDATES:
                diff = max_match - self.matches[name]
                self.bot_weights[name] = 0.25 ** diff

    def _select_candidates(self, valid, cur_m0):
        safe = [i for i in valid if cur_m0 + self.m[i] < self.W]
        pool = safe if safe else valid

        by_ratio = sorted(pool, key=lambda i: (self.v[i] / self.m[i], self.v[i]), reverse=True)[:15]
        by_val = sorted(pool, key=lambda i: (self.v[i], -self.m[i]), reverse=True)[:10]
        by_light = sorted(pool, key=lambda i: (self.m[i], -self.v[i]))[:10]

        seen = set()
        res = []
        for it in by_ratio + by_val + by_light:
            if it not in seen:
                seen.add(it)
                res.append(it)
        return res

    def choose_move(self, r, avail, M, V):
        valid = [i for i in avail if self.m[i] <= self.W - M[0]]
        if not valid:
            return 0
        if len(valid) == 1:
            return valid[0]

        total_w = sum(self.bot_weights.values())
        active_strats = [
            (name, func, self.bot_weights[name] / total_w)
            for name, func in BOT_CANDIDATES
            if self.bot_weights[name] / total_w > 0.01
        ]
        if not active_strats:
            best_name, best_func = max(BOT_CANDIDATES, key=lambda x: self.bot_weights[x[0]])
            active_strats = [(best_name, best_func, 1.0)]

        P1_POLICIES = [
            'greedy_ratio',
            'target_w_minus_1',
            'target_w_minus_2',
            'target_w_minus_3',
            'stay_light',
            'min_m',
            'greedy_v',
        ]

        candidates_to_eval = self._select_candidates(valid, M[0])

        # Распределяем оставшийся бюджет времени (лимит 10с -> ориентир 8.0с)
        rem_rounds = self.T - r + 1
        elapsed = time.perf_counter() - self.start_time
        round_budget = max(0.03, (8.0 - elapsed) / rem_rounds)
        round_deadline = time.perf_counter() + round_budget

        cand_evals = []
        for idx, cand in enumerate(candidates_to_eval):
            if idx >= 5 and time.perf_counter() > round_deadline:
                break

            if M[0] + self.m[cand] == self.W:
                cand_evals.append((-1.0, -1e9, -self.W, cand))
                continue

            surv_weight = 0.0
            exp_val = 0.0
            exp_margin = 0.0

            for b_name, b_func, b_w in active_strats:
                # В ранних раундах при большом числе гипотез подменяем медленный DP на быстрый прокси,
                # если бюджет времени подходит к концу
                use_func = b_func
                if b_name in ('knapsack_v', 'safe_knapsack_v') and (
                    len(active_strats) > 4 or time.perf_counter() > round_deadline
                ):
                    use_func = bot_safe_greedy_v if b_name == 'safe_knapsack_v' else bot_greedy_v_minm

                best_surv = False
                best_sc = -1
                best_margin = -10**9

                for pol in P1_POLICIES:
                    surv, sc, margin = rollout_game(
                        cand, r, self.T, self.K, self.W, self.m, self.v,
                        avail, M, V, use_func, pol
                    )
                    if (surv, sc, margin) > (best_surv, best_sc, best_margin):
                        best_surv, best_sc, best_margin = surv, sc, margin

                if best_surv:
                    surv_weight += b_w
                    exp_val += b_w * best_sc
                exp_margin += b_w * best_margin

            cand_evals.append((
                surv_weight,
                exp_val + 0.001 * (self.v[cand] / self.m[cand]),
                exp_margin,
                cand,
            ))

        if not cand_evals:
            safe = [i for i in valid if M[0] + self.m[i] < self.W]
            pool = safe if safe else valid
            return max(pool, key=lambda i: (self.v[i] / self.m[i], self.v[i], -i))

        # Приоритет: 1) вероятность выживания, 2) ожидаемая ценность, 3) запас массы
        cand_evals.sort(key=lambda x: (x[0], x[1], x[2]), reverse=True)
        best_surv, best_exp, best_margin, best_c = cand_evals[0]

        if best_surv <= 0:
            # Если в симуляциях выжить не удалось, выбираем ход с лучшим запасом массы,
            # а при равенстве — с большей плотностью ценности
            cand_evals.sort(key=lambda x: (x[2], self.v[x[3]] / self.m[x[3]], self.v[x[3]]), reverse=True)
            return cand_evals[0][3]

        return best_c


def read_ints(count):
    res = []
    while len(res) < count:
        line = sys.stdin.readline()
        if not line:
            break
        res.extend(map(int, line.split()))
    return res


def main():
    start_time = time.perf_counter()

    first_ints = read_ints(4)
    if len(first_ints) < 4:
        return
    K, N, T, W = first_ints

    m_vals = read_ints(N)
    v_vals = read_ints(N)
    if len(m_vals) < N or len(v_vals) < N:
        return

    m = [0] + m_vals
    v = [0] + v_vals

    agent = DwarfAgent(K, N, T, W, m, v, start_time)
    avail = set(range(1, N + 1))
    M = [0] * K
    V = [0] * K

    for r in range(1, T + 1):
        # 1. Наш ход
        a1 = agent.choose_move(r, avail, M, V)
        print(a1, flush=True)

        if a1 > 0:
            avail.remove(a1)
            M[0] += m[a1]
            V[0] += v[a1]

        # 2. Чтение ходов ботов
        bot_moves = read_ints(K - 1)
        if len(bot_moves) < K - 1:
            break

        for d in range(1, K):
            ad = bot_moves[d - 1]
            if ad > 0:
                if ad in avail:
                    avail.remove(ad)
                M[d] += m[ad]
                V[d] += v[ad]

        agent.observe_round([a1] + bot_moves)

    sys.exit(0)


if __name__ == '__main__':
    main()