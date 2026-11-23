import sys
import time


def solve_knapsack_all_caps(valid, cap_max, max_items, m, v, allow_2d=True):
    if cap_max <= 0 or max_items <= 0 or not valid:
        n = max(0, cap_max) + 1
        return [0] * n, [0] * n

    fit = [it for it in valid if m[it] <= cap_max]
    if not fit:
        return [0] * (cap_max + 1), [0] * (cap_max + 1)

    K_lim = min(max_items, cap_max, len(fit))
    if (not allow_2d) or K_lim >= min(cap_max, len(fit)) or K_lim > 12:
        dp = [0] * (cap_max + 1)
        mask = [0] * (cap_max + 1)
        for it in fit:
            wi, vi = m[it], v[it]
            bit = 1 << it
            for w in range(cap_max, wi - 1, -1):
                nv = dp[w - wi] + vi
                if nv > dp[w]:
                    dp[w] = nv
                    mask[w] = mask[w - wi] | bit
        best_masks = [0] * (cap_max + 1)
        best_vals = [0] * (cap_max + 1)
        bv = 0
        bm = 0
        for w in range(1, cap_max + 1):
            if dp[w] > bv:
                bv = dp[w]
                bm = mask[w]
            best_vals[w] = bv
            best_masks[w] = bm
        return best_masks, best_vals

    dp = [[0] * (cap_max + 1) for _ in range(K_lim + 1)]
    mask = [[0] * (cap_max + 1) for _ in range(K_lim + 1)]
    for it in fit:
        wi, vi = m[it], v[it]
        bit = 1 << it
        for k in range(K_lim, 0, -1):
            dp_k = dp[k]
            dp_prev = dp[k - 1]
            mask_k = mask[k]
            mask_prev = mask[k - 1]
            for w in range(cap_max, wi - 1, -1):
                nv = dp_prev[w - wi] + vi
                if nv > dp_k[w]:
                    dp_k[w] = nv
                    mask_k[w] = mask_prev[w - wi] | bit

    best_masks = [0] * (cap_max + 1)
    best_vals = [0] * (cap_max + 1)
    bv = 0
    bm = 0
    row_dp = dp[K_lim]
    row_mask = mask[K_lim]
    for w in range(1, cap_max + 1):
        if row_dp[w] > bv:
            bv = row_dp[w]
            bm = row_mask[w]
        best_vals[w] = bv
        best_masks[w] = bm
    return best_masks, best_vals


def solve_bot_knapsack(avail, cap, max_items, m, v):
    if cap <= 0 or max_items <= 0 or not avail:
        return []
    fit = [it for it in avail if m[it] <= cap]
    if not fit:
        return []
    if len(fit) <= max_items and sum(m[it] for it in fit) <= cap:
        return fit
    masks, _ = solve_knapsack_all_caps(
        fit, cap, max_items, m, v, allow_2d=(max_items <= 10 and len(fit) <= 50)
    )
    bm = masks[cap]
    return [it for it in fit if (bm >> it) & 1]


def mask_to_set(bm, valid):
    if bm == 0:
        return set()
    return {it for it in valid if (bm >> it) & 1}


def fast_pick(order, avail, cap, m):
    if cap <= 0:
        return 0
    for i in order:
        if i in avail and m[i] <= cap:
            return i
    return 0


def fast_safe_pick(order, avail, cur_m, target_limit, W, m):
    cap_target = min(target_limit, W - 1) - cur_m
    cap_under_w = W - 1 - cur_m
    cap_max = W - cur_m
    if cap_max <= 0:
        return 0
    fb_under_w = 0
    fb_max = 0
    for i in order:
        if i in avail:
            mi = m[i]
            if mi <= cap_target:
                return i
            if fb_under_w == 0 and mi <= cap_under_w:
                fb_under_w = i
            if fb_max == 0 and mi <= cap_max:
                fb_max = i
    return fb_under_w if fb_under_w else fb_max


def fast_plan_pick(plan_set, order_plan, order_fb, avail, cur_m, target_w, W, m):
    cap_plan = min(target_w, W - 1) - cur_m
    if cap_plan > 0 and plan_set:
        for i in order_plan:
            if i in plan_set and i in avail and m[i] <= cap_plan:
                return i
    return fast_safe_pick(order_fb, avail, cur_m, target_w, W, m)


class DwarfAgent:
    def __init__(self, K, N, T, W, m, v, start_time):
        self.K = K
        self.N = N
        self.T = T
        self.W = W
        self.m = m
        self.v = v
        self.start_time = start_time

        items = list(range(1, N + 1))
        self.ord_v_minm = sorted(items, key=lambda i: (v[i], -m[i], -i), reverse=True)
        self.ord_v_maxm = sorted(items, key=lambda i: (v[i], m[i], -i), reverse=True)
        self.ord_ratio_maxv = sorted(items, key=lambda i: (v[i] / m[i], v[i], -i), reverse=True)
        self.ord_ratio_minm = sorted(items, key=lambda i: (v[i] / m[i], -m[i], -i), reverse=True)
        self.ord_min_m = sorted(items, key=lambda i: (m[i], -v[i], i))
        self.ord_max_m = sorted(items, key=lambda i: (m[i], v[i], -i), reverse=True)
        self.ord_lin_1 = sorted(items, key=lambda i: (v[i] - m[i], v[i], -i), reverse=True)
        self.ord_lin_2 = sorted(items, key=lambda i: (v[i] - 2 * m[i], v[i], -i), reverse=True)

        self._init_bot_strategies()

        self.bot_weights = {name: prior for name, prior, _, _, _ in self.bot_specs}
        self.matches = {name: 0 for name, _, _, _, _ in self.bot_specs}
        self.total_obs = 0

        self.sim_avail = set(items)
        self.sim_M = [0] * K

    def _init_bot_strategies(self):
        W, m, v = self.W, self.m, self.v

        def make_greedy(order):
            return lambda av, cm, mo, rem: fast_pick(order, av, W - cm, m)

        def make_safe(order, target_w):
            return lambda av, cm, mo, rem: fast_safe_pick(order, av, cm, target_w, W, m)

        def make_beat(order):
            return lambda av, cm, mo, rem: fast_safe_pick(
                order, av, cm, (mo - 1) if mo > 0 else (W - 1), W, m
            )

        def make_knapsack(key_fn, reserve, use_rem):
            def fn(av, cm, mo, rem):
                cap = W - reserve - cm
                lim = rem if use_rem else 1000
                if cap > 0:
                    best = solve_bot_knapsack(av, cap, lim, m, v)
                    if best:
                        return max(best, key=key_fn)
                return fast_safe_pick(self.ord_v_minm, av, cm, W - 1, W, m)
            return fn

        key_v = lambda i: (v[i], -m[i], -i)
        key_ratio = lambda i: (v[i] / m[i], v[i], -i)

        g_v_minm = make_greedy(self.ord_v_minm)
        g_v_maxm = make_greedy(self.ord_v_maxm)
        g_ratio_maxv = make_greedy(self.ord_ratio_maxv)
        g_ratio_minm = make_greedy(self.ord_ratio_minm)
        s_v = make_safe(self.ord_v_minm, W - 1)
        s_ratio = make_safe(self.ord_ratio_maxv, W - 1)
        s2_ratio = make_safe(self.ord_ratio_maxv, W - 2)
        b_v = make_beat(self.ord_v_minm)
        b_ratio = make_beat(self.ord_ratio_maxv)
        l_1 = make_safe(self.ord_lin_1, W - 1)
        l_2 = make_safe(self.ord_lin_2, W - 1)
        g_min_m = make_greedy(self.ord_min_m)
        g_max_m = make_greedy(self.ord_max_m)

        # (name, prior, exact_fn, rollout_fn, is_expensive)
        self.bot_specs = [
            ('greedy_v_minm', 1.0, g_v_minm, g_v_minm, False),
            ('greedy_v_maxm', 1.0, g_v_maxm, g_v_maxm, False),
            ('greedy_ratio_maxv', 1.0, g_ratio_maxv, g_ratio_maxv, False),
            ('greedy_ratio_minm', 1.0, g_ratio_minm, g_ratio_minm, False),
            ('safe_greedy_v', 1.0, s_v, s_v, False),
            ('safe_greedy_ratio', 1.0, s_ratio, s_ratio, False),
            ('safe2_greedy_ratio', 1.0, s2_ratio, s2_ratio, False),
            ('beat_max_v', 1.0, b_v, b_v, False),
            ('beat_max_ratio', 1.0, b_ratio, b_ratio, False),
            ('lin_1', 1.0, l_1, l_1, False),
            ('lin_2', 1.0, l_2, l_2, False),
            ('knapsack_v', 1.0, make_knapsack(key_v, 0, False), g_v_minm, True),
            ('knapsack_ratio', 1.0, make_knapsack(key_ratio, 0, False), g_ratio_maxv, True),
            ('safe_knapsack_v', 1.0, make_knapsack(key_v, 1, False), s_v, True),
            ('safe_knapsack_lim_v', 1.0, make_knapsack(key_v, 1, True), s_v, True),
            ('greedy_min_m', 0.05, g_min_m, g_min_m, False),
            ('greedy_max_m', 0.05, g_max_m, g_max_m, False),
        ]

    def observe_round(self, r, r_moves):
        rem_r = self.T - r + 1
        p1_m = r_moves[0]
        if p1_m > 0 and p1_m in self.sim_avail:
            self.sim_avail.remove(p1_m)
            self.sim_M[0] += self.m[p1_m]

        cur_max_match = max(self.matches.values()) if self.total_obs > 0 else 0
        time_tight = (time.perf_counter() - self.start_time) > 4.5

        for d in range(1, self.K):
            ad = r_moves[d]
            self.total_obs += 1
            max_other_d = max((self.sim_M[j] for j in range(self.K) if j != d), default=0)
            for name, _, exact_fn, _, is_exp in self.bot_specs:
                if is_exp and (time_tight or (cur_max_match - self.matches[name] >= 3)):
                    continue
                if exact_fn(self.sim_avail, self.sim_M[d], max_other_d, rem_r) == ad:
                    self.matches[name] += 1
            if ad > 0:
                if ad in self.sim_avail:
                    self.sim_avail.remove(ad)
                self.sim_M[d] += self.m[ad]

        if self.total_obs > 0:
            max_match = max(self.matches.values())
            decay = 0.2 if max_match == self.total_obs else 0.45
            for name, prior, _, _, _ in self.bot_specs:
                diff = max_match - self.matches[name]
                self.bot_weights[name] = prior * (decay ** diff)

    def _rollout(self, start_cand, cur_r, init_avail, init_M, init_V, bot_fn, pol_name, plans):
        avail = set(init_avail)
        M = list(init_M)
        V = list(init_V)
        K, T, W, m, v = self.K, self.T, self.W, self.m, self.v
        plan_w1, plan_w2, plan_w4 = plans

        if start_cand > 0:
            avail.remove(start_cand)
            M[0] += m[start_cand]
            V[0] += v[start_cand]

        rem_cur = T - cur_r + 1
        for d in range(1, K):
            max_other_d = max((M[j] for j in range(K) if j != d), default=0)
            ad = bot_fn(avail, M[d], max_other_d, rem_cur)
            if ad > 0:
                avail.remove(ad)
                M[d] += m[ad]
                V[d] += v[ad]

        ord_ratio = self.ord_ratio_maxv
        ord_v = self.ord_v_minm
        ord_minm = self.ord_min_m

        for fut_r in range(cur_r + 1, T + 1):
            if not avail:
                break
            rem_f = T - fut_r + 1

            if pol_name == 'knapsack_w1':
                fut_a = fast_plan_pick(plan_w1, ord_ratio, ord_ratio, avail, M[0], W - 1, W, m)
            elif pol_name == 'knapsack_w2':
                fut_a = fast_plan_pick(plan_w2, ord_ratio, ord_ratio, avail, M[0], W - 2, W, m)
            elif pol_name == 'knapsack_w4':
                fut_a = fast_plan_pick(plan_w4, ord_ratio, ord_ratio, avail, M[0], max(1, W - 4), W, m)
            elif pol_name == 'greedy_ratio':
                fut_a = fast_safe_pick(ord_ratio, avail, M[0], W - 1, W, m)
            elif pol_name == 'greedy_v':
                fut_a = fast_safe_pick(ord_v, avail, M[0], W - 1, W, m)
            elif pol_name == 'stay_light':
                max_bots = max(M[1:]) if K > 1 else 0
                if max_bots > 0:
                    fut_a = fast_pick(ord_ratio, avail, min(max_bots - 1, W - 1) - M[0], m)
                    if fut_a == 0:
                        fut_a = fast_pick(ord_minm, avail, W - M[0], m)
                else:
                    fut_a = fast_pick(ord_minm, avail, W - M[0], m)
            elif pol_name == 'min_m':
                fut_a = fast_pick(ord_minm, avail, W - M[0], m)
            else:
                fut_a = 0

            if fut_a > 0:
                avail.remove(fut_a)
                M[0] += m[fut_a]
                V[0] += v[fut_a]

            for d in range(1, K):
                max_other_d = max((M[j] for j in range(K) if j != d), default=0)
                ad = bot_fn(avail, M[d], max_other_d, rem_f)
                if ad > 0:
                    avail.remove(ad)
                    M[d] += m[ad]
                    V[d] += v[ad]

        max_other = max(M[1:]) if K > 1 else 0
        survived = M[0] < max_other
        return survived, (V[0] if survived else 0), max_other - M[0]

    def choose_move(self, r, avail, M, V):
        valid = [i for i in avail if self.m[i] <= self.W - M[0]]
        if not valid:
            return 0
        if len(valid) == 1:
            return valid[0]

        safe = [i for i in valid if M[0] + self.m[i] < self.W]
        pool = safe if safe else valid

        # Жёсткий аварийный порог по времени (гарантия отсутствия TL/PE)
        elapsed = time.perf_counter() - self.start_time
        if elapsed > 6.0:
            return max(pool, key=lambda i: (self.v[i] / self.m[i], self.v[i], -i))

        rem_rounds = self.T - r + 1
        cap_max = max(0, self.W - 1 - M[0])
        best_masks, _ = solve_knapsack_all_caps(valid, cap_max, rem_rounds, self.m, self.v)

        knap_base = set()
        if cap_max > 0:
            for c_off in (0, 1, 3):
                c = max(1, cap_max - c_off)
                knap_base |= mask_to_set(best_masks[c], valid)

        by_knap = [i for i in self.ord_ratio_maxv if i in knap_base and i in pool]
        by_ratio = sorted(pool, key=lambda i: (self.v[i] / self.m[i], self.v[i]), reverse=True)[:14]
        by_val = sorted(pool, key=lambda i: (self.v[i], -self.m[i]), reverse=True)[:10]
        by_light = sorted(pool, key=lambda i: (self.m[i], -self.v[i]))[:8]

        seen = set()
        candidates_to_eval = []
        for it in by_knap + by_ratio + by_val + by_light:
            if it not in seen:
                seen.add(it)
                candidates_to_eval.append(it)

        # Объединяем веса стратегий, использующих одинаковую функцию в роллаутах
        total_w = sum(self.bot_weights.values())
        merged_fns = {}
        for name, _, _, rollout_fn, _ in self.bot_specs:
            w_norm = self.bot_weights[name] / total_w
            if w_norm > 0.015:
                merged_fns[rollout_fn] = merged_fns.get(rollout_fn, 0.0) + w_norm

        if not merged_fns:
            best_spec = max(self.bot_specs, key=lambda x: self.bot_weights[x[0]])
            merged_fns[best_spec[3]] = 1.0

        active_rollout_strats = list(merged_fns.items())

        if r == self.T:
            policies = ['greedy_ratio']
        elif len(active_rollout_strats) > 6:
            policies = ['knapsack_w1', 'knapsack_w2', 'stay_light', 'min_m']
        else:
            policies = [
                'knapsack_w1',
                'knapsack_w2',
                'knapsack_w4',
                'greedy_ratio',
                'greedy_v',
                'stay_light',
                'min_m',
            ]

        round_budget = max(0.02, (5.2 - elapsed) / rem_rounds)
        round_deadline = time.perf_counter() + round_budget

        cand_evals = []
        for idx, cand in enumerate(candidates_to_eval):
            if idx >= 3 and time.perf_counter() > round_deadline:
                break

            mc = self.m[cand]
            if M[0] + mc == self.W:
                cand_evals.append((-1.0, -1.0, -self.W, cand))
                continue

            rem_cap = cap_max - mc
            if rem_cap > 0:
                p1 = mask_to_set(best_masks[rem_cap], valid)
                p1.discard(cand)
                p2 = mask_to_set(best_masks[max(0, rem_cap - 1)], valid)
                p2.discard(cand)
                p4 = mask_to_set(best_masks[max(0, rem_cap - 3)], valid)
                p4.discard(cand)
                plans = (p1, p2, p4)
            else:
                plans = (set(), set(), set())

            exp_adapt = 0.0
            pol_scores = {pol: 0.0 for pol in policies}
            surv_weight = 0.0
            exp_margin = 0.0

            for bot_fn, b_w in active_rollout_strats:
                best_surv = False
                best_sc = -1
                best_margin = -10**9

                for pol in policies:
                    surv, sc, margin = self._rollout(
                        cand, r, avail, M, V, bot_fn, pol, plans
                    )
                    pol_scores[pol] += b_w * sc
                    if (surv, sc, margin) > (best_surv, best_sc, best_margin):
                        best_surv, best_sc, best_margin = surv, sc, margin

                if best_surv:
                    surv_weight += b_w
                    exp_adapt += b_w * best_sc
                exp_margin += b_w * best_margin

            exp_fixed = max(pol_scores.values())
            blended_score = 0.7 * exp_adapt + 0.3 * exp_fixed

            cand_evals.append((
                blended_score + 1e-4 * (self.v[cand] / mc),
                surv_weight,
                exp_margin,
                cand,
            ))

        if not cand_evals:
            return max(pool, key=lambda i: (self.v[i] / self.m[i], self.v[i], -i))

        cand_evals.sort(key=lambda x: (x[0], x[1], x[2]), reverse=True)
        best_score, best_surv, _, best_c = cand_evals[0]

        if best_surv <= 0 and best_score < 0.01:
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
        try:
            a1 = agent.choose_move(r, avail, M, V)
        except Exception:
            valid_fb = [i for i in avail if m[i] <= W - M[0]]
            a1 = max(valid_fb, key=lambda i: (v[i] / m[i], v[i], -i)) if valid_fb else 0

        # Гарантируем корректность хода перед выводом
        if a1 == 0:
            valid_chk = [i for i in avail if m[i] <= W - M[0]]
            if valid_chk:
                a1 = valid_chk[0]
        elif a1 not in avail or m[a1] > W - M[0]:
            valid_chk = [i for i in avail if m[i] <= W - M[0]]
            a1 = valid_chk[0] if valid_chk else 0

        print(a1, flush=True)

        if a1 > 0:
            avail.remove(a1)
            M[0] += m[a1]
            V[0] += v[a1]

        bot_moves = []
        while True:
            line = sys.stdin.readline()
            if not line:
                break
            parts = line.split()
            if parts:
                bot_moves = list(map(int, parts))
                break

        if len(bot_moves) < K - 1:
            break

        for d in range(1, K):
            ad = bot_moves[d - 1]
            if ad > 0:
                if ad in avail:
                    avail.remove(ad)
                M[d] += m[ad]
                V[d] += v[ad]

        try:
            agent.observe_round(r, [a1] + bot_moves)
        except Exception:
            pass

    sys.exit(0)


if __name__ == '__main__':
    main()