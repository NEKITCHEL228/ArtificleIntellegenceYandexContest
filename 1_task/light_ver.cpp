#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace std;

using Mask = unsigned __int128;
const Mask ONE = 1;

enum BotArchetype {
    BOT_RATIO = 0,
    BOT_V,
    BOT_SAFE_W1,
    BOT_SAFE_W2,
    BOT_BEAT_MAX,
    BOT_MIN_M,
    ARCH_COUNT
};

enum P1Policy {
    POL_TARGET_W1 = 0,
    POL_TARGET_W2,
    POL_TARGET_W3,
    POL_STAY_LIGHT,
    POL_MIN_M,
    POL_COUNT
};

int K, N, T, W;
vector<int> item_m;
vector<int> item_v;
vector<int> order_ratio;
vector<int> order_v;
vector<int> order_min_m;

int bot_pick(BotArchetype arch, Mask avail, int cur_m, int max_other) {
    int cap = W - cur_m;
    if (cap <= 0 || avail == 0) return 0;

    switch (arch) {
        case BOT_RATIO: {
            for (int id : order_ratio) {
                if (((avail >> id) & ONE) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BOT_V: {
            for (int id : order_v) {
                if (((avail >> id) & ONE) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BOT_SAFE_W1: {
            int safe_cap = min(cap, W - 1 - cur_m);
            if (safe_cap > 0) {
                for (int id : order_ratio) {
                    if (((avail >> id) & ONE) && item_m[id] <= safe_cap) return id;
                }
            }
            for (int id : order_ratio) {
                if (((avail >> id) & ONE) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BOT_SAFE_W2: {
            int safe_cap = min(cap, max(1, W - 2) - cur_m);
            if (safe_cap > 0) {
                for (int id : order_ratio) {
                    if (((avail >> id) & ONE) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w = min(cap, W - 1 - cur_m);
            if (safe_w > 0) {
                for (int id : order_ratio) {
                    if (((avail >> id) & ONE) && item_m[id] <= safe_w) return id;
                }
            }
            for (int id : order_ratio) {
                if (((avail >> id) & ONE) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BOT_BEAT_MAX: {
            int limit = (max_other > 0 ? max_other : W);
            int safe_cap = min(cap, limit - 1 - cur_m);
            if (safe_cap > 0) {
                for (int id : order_v) {
                    if (((avail >> id) & ONE) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w = min(cap, W - 1 - cur_m);
            if (safe_w > 0) {
                for (int id : order_v) {
                    if (((avail >> id) & ONE) && item_m[id] <= safe_w) return id;
                }
            }
            for (int id : order_v) {
                if (((avail >> id) & ONE) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BOT_MIN_M: {
            for (int id : order_min_m) {
                if (((avail >> id) & ONE) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        default: return 0;
    }
}

struct RolloutRes {
    bool survived;
    int score;
    int margin;
};

RolloutRes simulate_rollout(Mask avail, vector<int> M, vector<int> V,
                             int start_round, BotArchetype btype, P1Policy pol) {
    for (int r = start_round; r <= T; ++r) {
        if (avail == 0) break;

        int cap0 = W - M[0];
        int fut_a = 0;
        if (cap0 > 0) {
            int safe_lim = W;
            if (pol == POL_TARGET_W1) safe_lim = W - 1;
            else if (pol == POL_TARGET_W2) safe_lim = max(1, W - 2);
            else if (pol == POL_TARGET_W3) safe_lim = max(1, W - 3);
            else if (pol == POL_STAY_LIGHT) {
                int max_b = 0;
                for (int j = 1; j < K; ++j) max_b = max(max_b, M[j]);
                safe_lim = max(0, max_b - 1);
            }

            if (pol != POL_MIN_M) {
                int safe_cap = min(cap0, safe_lim - M[0]);
                if (safe_cap > 0) {
                    for (int id : order_ratio) {
                        if (((avail >> id) & ONE) && item_m[id] <= safe_cap) {
                            fut_a = id;
                            break;
                        }
                    }
                }
            }
            if (fut_a == 0) {
                for (int id : order_min_m) {
                    if (((avail >> id) & ONE) && item_m[id] <= cap0) {
                        fut_a = id;
                        break;
                    }
                }
            }
            if (fut_a > 0) {
                avail &= ~(ONE << fut_a);
                M[0] += item_m[fut_a];
                V[0] += item_v[fut_a];
            }
        }

        // Bots move
        for (int d = 1; d < K; ++d) {
            int max_other = 0;
            for (int j = 0; j < K; ++j) if (j != d) max_other = max(max_other, M[j]);
            int pick = bot_pick(btype, avail, M[d], max_other);
            if (pick > 0) {
                avail &= ~(ONE << pick);
                M[d] += item_m[pick];
                V[d] += item_v[pick];
            }
        }
    }

    int max_other = 0;
    for (int j = 1; j < K; ++j) max_other = max(max_other, M[j]);
    bool survived = (M[0] < max_other);
    return {survived, survived ? V[0] : 0, max_other - M[0]};
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> K >> N >> T >> W)) return 0;

    item_m.assign(N + 1, 0);
    item_v.assign(N + 1, 0);
    for (int i = 1; i <= N; ++i) cin >> item_m[i];
    for (int i = 1; i <= N; ++i) cin >> item_v[i];

    vector<int> all_items(N);
    iota(all_items.begin(), all_items.end(), 1);

    order_ratio = all_items;
    sort(order_ratio.begin(), order_ratio.end(), [](int a, int b) {
        double ra = (double)item_v[a] / item_m[a];
        double rb = (double)item_v[b] / item_m[b];
        if (abs(ra - rb) > 1e-9) return ra > rb;
        if (item_v[a] != item_v[b]) return item_v[a] > item_v[b];
        return a < b;
    });

    order_v = all_items;
    sort(order_v.begin(), order_v.end(), [](int a, int b) {
        if (item_v[a] != item_v[b]) return item_v[a] > item_v[b];
        if (item_m[a] != item_m[b]) return item_m[a] < item_m[b];
        return a < b;
    });

    order_min_m = all_items;
    sort(order_min_m.begin(), order_min_m.end(), [](int a, int b) {
        if (item_m[a] != item_m[b]) return item_m[a] < item_m[b];
        if (item_v[a] != item_v[b]) return item_v[a] > item_v[b];
        return a < b;
    });

    Mask avail = 0;
    for (int i = 1; i <= N; ++i) avail |= (ONE << i);

    vector<int> M(K, 0);
    vector<int> V(K, 0);

    vector<int> bot_matches(ARCH_COUNT, 0);
    vector<double> bot_w(ARCH_COUNT, 1.0);

    Mask sim_avail = avail;
    vector<int> sim_M = M;

    auto update_bot_obs = [&](const vector<int>& round_moves) {
        int p1 = round_moves[0];
        if (p1 > 0 && ((sim_avail >> p1) & ONE)) {
            sim_avail &= ~(ONE << p1);
            sim_M[0] += item_m[p1];
        }
        for (int d = 1; d < K; ++d) {
            int ad = round_moves[d];
            int max_other = 0;
            for (int j = 0; j < K; ++j) if (j != d) max_other = max(max_other, sim_M[j]);
            for (int b = 0; b < ARCH_COUNT; ++b) {
                if (bot_pick((BotArchetype)b, sim_avail, sim_M[d], max_other) == ad) {
                    bot_matches[b]++;
                }
            }
            if (ad > 0) {
                if ((sim_avail >> ad) & ONE) sim_avail &= ~(ONE << ad);
                sim_M[d] += item_m[ad];
            }
        }

        int max_m = *max_element(bot_matches.begin(), bot_matches.end());
        double sum_w = 0.0;
        for (int b = 0; b < ARCH_COUNT; ++b) {
            bot_w[b] = pow(0.25, max_m - bot_matches[b]);
            sum_w += bot_w[b];
        }
        for (int b = 0; b < ARCH_COUNT; ++b) {
            bot_w[b] /= sum_w;
        }
    };

    const P1Policy POLS[] = {POL_TARGET_W1, POL_TARGET_W2, POL_TARGET_W3, POL_STAY_LIGHT, POL_MIN_M};

    for (int r = 1; r <= T; ++r) {
        int cap0 = W - M[0];
        vector<int> valid;
        for (int id = 1; id <= N; ++id) {
            if (((avail >> id) & ONE) && item_m[id] <= cap0) valid.push_back(id);
        }

        int a1 = 0;
        if (!valid.empty()) {
            if (valid.size() == 1) {
                a1 = valid[0];
            } else {
                vector<pair<BotArchetype, double>> active_bots;
                for (int b = 0; b < ARCH_COUNT; ++b) {
                    if (bot_w[b] > 0.02) active_bots.push_back({(BotArchetype)b, bot_w[b]});
                }
                if (active_bots.empty()) {
                    int best_b = 0;
                    for (int b = 1; b < ARCH_COUNT; ++b) if (bot_w[b] > bot_w[best_b]) best_b = b;
                    active_bots.push_back({(BotArchetype)best_b, 1.0});
                }

                // Collect candidates
                vector<int> cands;
                vector<bool> seen(N + 1, false);

                int lim_ratio = min((int)valid.size(), 8);
                int cnt = 0;
                for (int id : order_ratio) {
                    if (((avail >> id) & ONE) && item_m[id] <= cap0) {
                        if (!seen[id]) { seen[id] = true; cands.push_back(id); }
                        if (++cnt >= lim_ratio) break;
                    }
                }

                int lim_v = min((int)valid.size(), 4);
                cnt = 0;
                for (int id : order_v) {
                    if (((avail >> id) & ONE) && item_m[id] <= cap0) {
                        if (!seen[id]) { seen[id] = true; cands.push_back(id); }
                        if (++cnt >= lim_v) break;
                    }
                }

                int lim_m = min((int)valid.size(), 2);
                cnt = 0;
                for (int id : order_min_m) {
                    if (((avail >> id) & ONE) && item_m[id] <= cap0) {
                        if (!seen[id]) { seen[id] = true; cands.push_back(id); }
                        if (++cnt >= lim_m) break;
                    }
                }

                struct CandEval {
                    double surv_w;
                    double exp_v;
                    double exp_margin;
                    int cand;
                };
                vector<CandEval> evals;

                for (int cand : cands) {
                    if (M[0] + item_m[cand] == W) continue;

                    double surv_w = 0.0;
                    double exp_v = 0.0;
                    double exp_m = 0.0;

                    for (auto& ab : active_bots) {
                        BotArchetype btype = ab.first;
                        double bw = ab.second;

                        Mask cand_avail = avail & ~(ONE << cand);
                        vector<int> cand_M = M;
                        vector<int> cand_V = V;
                        cand_M[0] += item_m[cand];
                        cand_V[0] += item_v[cand];

                        for (int d = 1; d < K; ++d) {
                            int mo = 0;
                            for (int j = 0; j < K; ++j) if (j != d) mo = max(mo, cand_M[j]);
                            int pick = bot_pick(btype, cand_avail, cand_M[d], mo);
                            if (pick > 0) {
                                cand_avail &= ~(ONE << pick);
                                cand_M[d] += item_m[pick];
                                cand_V[d] += item_v[pick];
                            }
                        }

                        bool best_surv = false;
                        int best_sc = -1;
                        int best_margin = -1e9;

                        for (P1Policy pol : POLS) {
                            RolloutRes res = simulate_rollout(cand_avail, cand_M, cand_V, r + 1, btype, pol);
                            if (make_tuple(res.survived, res.score, res.margin) >
                                make_tuple(best_surv, best_sc, best_margin)) {
                                best_surv = res.survived;
                                best_sc = res.score;
                                best_margin = res.margin;
                            }
                        }

                        if (best_surv) {
                            surv_w += bw;
                            exp_v += bw * best_sc;
                        }
                        exp_m += bw * best_margin;
                    }

                    evals.push_back({
                        surv_w,
                        exp_v + 0.001 * ((double)item_v[cand] / item_m[cand]),
                        exp_m,
                        cand
                    });
                }

                if (evals.empty()) {
                    a1 = valid[0];
                } else {
                    sort(evals.begin(), evals.end(), [](const CandEval& a, const CandEval& b) {
                        if (a.surv_w != b.surv_w) return a.surv_w > b.surv_w;
                        if (a.exp_v != b.exp_v) return a.exp_v > b.exp_v;
                        if (a.exp_margin != b.exp_margin) return a.exp_margin > b.exp_margin;
                        return a.cand < b.cand;
                    });

                    if (evals[0].surv_w <= 0.0) {
                        sort(evals.begin(), evals.end(), [](const CandEval& a, const CandEval& b) {
                            if (a.exp_margin != b.exp_margin) return a.exp_margin > b.exp_margin;
                            double ra = (double)item_v[a.cand] / item_m[a.cand];
                            double rb = (double)item_v[b.cand] / item_m[b.cand];
                            if (ra != rb) return ra > rb;
                            return a.cand < b.cand;
                        });
                    }
                    a1 = evals[0].cand;
                }
            }
        }

        // Fail-safe check
        if (a1 < 0 || a1 > N || (a1 > 0 && (!((avail >> a1) & ONE) || item_m[a1] > cap0))) {
            a1 = valid.empty() ? 0 : valid[0];
        }

        cout << a1 << endl; // Flushes output immediately

        if (a1 > 0) {
            avail &= ~(ONE << a1);
            M[0] += item_m[a1];
            V[0] += item_v[a1];
        }

        vector<int> bot_moves(K - 1);
        bool read_ok = true;
        for (int d = 1; d < K; ++d) {
            if (!(cin >> bot_moves[d - 1])) {
                read_ok = false;
                break;
            }
        }
        if (!read_ok) break;

        for (int d = 1; d < K; ++d) {
            int ad = bot_moves[d - 1];
            if (ad > 0) {
                if ((avail >> ad) & ONE) avail &= ~(ONE << ad);
                M[d] += item_m[ad];
                V[d] += item_v[ad];
            }
        }

        vector<int> round_moves = {a1};
        round_moves.insert(round_moves.end(), bot_moves.begin(), bot_moves.end());
        update_bot_obs(round_moves);
    }

    return 0;
}
