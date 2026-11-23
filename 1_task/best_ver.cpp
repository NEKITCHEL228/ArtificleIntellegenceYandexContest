#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <numeric>
#include <chrono>

using namespace std;

using Mask = unsigned __int128;
static const Mask ONE = 1;

int K, N, T, W;
vector<int> item_m;
vector<int> item_v;

vector<int> order_v_minm;
vector<int> order_v_maxm;
vector<int> order_ratio_maxv;
vector<int> order_ratio_minm;
vector<int> order_min_m;

enum class BotType {
    GREEDY_V_MINM,
    GREEDY_V_MAXM,
    GREEDY_RATIO_MAXV,
    GREEDY_RATIO_MINM,
    SAFE_W1_RATIO,
    SAFE_W1_V,
    SAFE_W2_RATIO,
    SAFE_W2_V,
    SAFE_W3_RATIO,
    SAFE_W3_V,
    BEAT_MAX_V,
    BEAT_MAX_RATIO,
    MIN_M,
    NUM_BOTS
};

const int BOT_COUNT = (int)BotType::NUM_BOTS;

inline int bot_pick(BotType type, Mask avail, int cur_m, int max_other) {
    int cap = W - cur_m;
    if (cap <= 0) return 0;

    switch (type) {
        case BotType::GREEDY_V_MINM: {
            for (int id : order_v_minm) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::GREEDY_V_MAXM: {
            for (int id : order_v_maxm) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::GREEDY_RATIO_MAXV: {
            for (int id : order_ratio_maxv) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::GREEDY_RATIO_MINM: {
            for (int id : order_ratio_minm) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::SAFE_W1_RATIO: {
            int safe_cap = min(cap, W - 1 - cur_m);
            if (safe_cap > 0) {
                for (int id : order_ratio_maxv) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            for (int id : order_ratio_maxv) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::SAFE_W1_V: {
            int safe_cap = min(cap, W - 1 - cur_m);
            if (safe_cap > 0) {
                for (int id : order_v_minm) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            for (int id : order_v_minm) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::SAFE_W2_RATIO: {
            int safe_cap = min(cap, max(1, W - 2) - cur_m);
            if (safe_cap > 0) {
                for (int id : order_ratio_maxv) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w1 = min(cap, W - 1 - cur_m);
            if (safe_w1 > 0) {
                for (int id : order_ratio_maxv) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_w1) return id;
                }
            }
            for (int id : order_ratio_maxv) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::SAFE_W2_V: {
            int safe_cap = min(cap, max(1, W - 2) - cur_m);
            if (safe_cap > 0) {
                for (int id : order_v_minm) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w1 = min(cap, W - 1 - cur_m);
            if (safe_w1 > 0) {
                for (int id : order_v_minm) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_w1) return id;
                }
            }
            for (int id : order_v_minm) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::SAFE_W3_RATIO: {
            int safe_cap = min(cap, max(1, W - 3) - cur_m);
            if (safe_cap > 0) {
                for (int id : order_ratio_maxv) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w2 = min(cap, max(1, W - 2) - cur_m);
            if (safe_w2 > 0) {
                for (int id : order_ratio_maxv) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_w2) return id;
                }
            }
            for (int id : order_ratio_maxv) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::SAFE_W3_V: {
            int safe_cap = min(cap, max(1, W - 3) - cur_m);
            if (safe_cap > 0) {
                for (int id : order_v_minm) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w2 = min(cap, max(1, W - 2) - cur_m);
            if (safe_w2 > 0) {
                for (int id : order_v_minm) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_w2) return id;
                }
            }
            for (int id : order_v_minm) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::BEAT_MAX_V: {
            int limit = (max_other > 0) ? max_other : W;
            int safe_cap = min(cap, limit - 1 - cur_m);
            if (safe_cap > 0) {
                for (int id : order_v_minm) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w1 = min(cap, W - 1 - cur_m);
            if (safe_w1 > 0) {
                for (int id : order_v_minm) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_w1) return id;
                }
            }
            for (int id : order_v_minm) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::BEAT_MAX_RATIO: {
            int limit = (max_other > 0) ? max_other : W;
            int safe_cap = min(cap, limit - 1 - cur_m);
            if (safe_cap > 0) {
                for (int id : order_ratio_maxv) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_cap) return id;
                }
            }
            int safe_w1 = min(cap, W - 1 - cur_m);
            if (safe_w1 > 0) {
                for (int id : order_ratio_maxv) {
                    if (((avail >> id) & 1) && item_m[id] <= safe_w1) return id;
                }
            }
            for (int id : order_ratio_maxv) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        case BotType::MIN_M: {
            for (int id : order_min_m) {
                if (((avail >> id) & 1) && item_m[id] <= cap) return id;
            }
            return 0;
        }
        default:
            return 0;
    }
}

// Fast forward projection of heaviest bot weight
inline int project_bot_max(Mask cur_avail, const vector<int>& cur_M, BotType btype, int start_r) {
    Mask avail = cur_avail;
    vector<int> M = cur_M;
    for (int r = start_r; r <= T; ++r) {
        if (!avail) break;
        for (int d = 1; d < K; ++d) {
            int max_other = 0;
            for (int j = 0; j < K; ++j) if (j != d) max_other = max(max_other, M[j]);
            int pick = bot_pick(btype, avail, M[d], max_other);
            if (pick > 0) {
                avail &= ~(ONE << pick);
                M[d] += item_m[pick];
            }
        }
    }
    int max_b = 0;
    for (int j = 1; j < K; ++j) max_b = max(max_b, M[j]);
    return max_b;
}

struct RolloutResult {
    bool survived;
    int score;
    int margin;
};

enum class P1Policy {
    DYNAMIC_TARGET_1, // target = min(W-1, bot_max - 1)
    DYNAMIC_TARGET_2, // target = min(W-1, bot_max - 2)
    DYNAMIC_TARGET_3, // target = min(W-1, bot_max - 3)
    GREEDY_RATIO,
    STAY_LIGHT,
    MIN_M,
    GREEDY_V,
    NUM_POLICIES
};

inline RolloutResult simulate_rollout(
    Mask avail, vector<int> M, vector<int> V,
    int start_round, int projected_bot_max,
    BotType btype, P1Policy pol
) {
    int target_W = W - 1;
    if (pol == P1Policy::DYNAMIC_TARGET_1) {
        target_W = min(W - 1, max(1, projected_bot_max - 1));
    } else if (pol == P1Policy::DYNAMIC_TARGET_2) {
        target_W = min(W - 1, max(1, projected_bot_max - 2));
    } else if (pol == P1Policy::DYNAMIC_TARGET_3) {
        target_W = min(W - 1, max(1, projected_bot_max - 3));
    }

    for (int r = start_round; r <= T; ++r) {
        if (!avail) break;

        int cap0 = W - M[0];
        int fut_a = 0;

        if (cap0 > 0) {
            switch (pol) {
                case P1Policy::DYNAMIC_TARGET_1:
                case P1Policy::DYNAMIC_TARGET_2:
                case P1Policy::DYNAMIC_TARGET_3: {
                    int safe_cap = min(cap0, target_W - M[0]);
                    if (safe_cap > 0) {
                        for (int id : order_ratio_maxv) {
                            if (((avail >> id) & 1) && item_m[id] <= safe_cap) { fut_a = id; break; }
                        }
                    }
                    if (fut_a == 0) {
                        int safe_w = min(cap0, W - 1 - M[0]);
                        if (safe_w > 0) {
                            for (int id : order_ratio_maxv) {
                                if (((avail >> id) & 1) && item_m[id] <= safe_w) { fut_a = id; break; }
                            }
                        }
                    }
                    if (fut_a == 0) {
                        for (int id : order_ratio_maxv) {
                            if (((avail >> id) & 1) && item_m[id] <= cap0) { fut_a = id; break; }
                        }
                    }
                    break;
                }
                case P1Policy::GREEDY_RATIO: {
                    int safe_cap = min(cap0, W - 1 - M[0]);
                    if (safe_cap > 0) {
                        for (int id : order_ratio_maxv) {
                            if (((avail >> id) & 1) && item_m[id] <= safe_cap) { fut_a = id; break; }
                        }
                    }
                    if (fut_a == 0) {
                        for (int id : order_ratio_maxv) {
                            if (((avail >> id) & 1) && item_m[id] <= cap0) { fut_a = id; break; }
                        }
                    }
                    break;
                }
                case P1Policy::STAY_LIGHT: {
                    int max_b = 0;
                    for (int j = 1; j < K; ++j) max_b = max(max_b, M[j]);
                    int safe_cap = min(cap0, max_b - M[0]);
                    if (safe_cap > 0) {
                        for (int id : order_ratio_maxv) {
                            if (((avail >> id) & 1) && item_m[id] <= safe_cap) { fut_a = id; break; }
                        }
                    }
                    if (fut_a == 0) {
                        for (int id : order_min_m) {
                            if (((avail >> id) & 1) && item_m[id] <= cap0) { fut_a = id; break; }
                        }
                    }
                    break;
                }
                case P1Policy::MIN_M: {
                    for (int id : order_min_m) {
                        if (((avail >> id) & 1) && item_m[id] <= cap0) { fut_a = id; break; }
                    }
                    break;
                }
                case P1Policy::GREEDY_V: {
                    int safe_cap = min(cap0, W - 1 - M[0]);
                    if (safe_cap > 0) {
                        for (int id : order_v_minm) {
                            if (((avail >> id) & 1) && item_m[id] <= safe_cap) { fut_a = id; break; }
                        }
                    }
                    if (fut_a == 0) {
                        for (int id : order_v_minm) {
                            if (((avail >> id) & 1) && item_m[id] <= cap0) { fut_a = id; break; }
                        }
                    }
                    break;
                }
                default: break;
            }

            if (fut_a > 0) {
                avail &= ~(ONE << fut_a);
                M[0] += item_m[fut_a];
                V[0] += item_v[fut_a];
            }
        }

        // bots move
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

    auto global_start_time = chrono::steady_clock::now();

    if (!(cin >> K >> N >> T >> W)) return 0;

    item_m.assign(N + 1, 0);
    item_v.assign(N + 1, 0);

    for (int i = 1; i <= N; ++i) cin >> item_m[i];
    for (int i = 1; i <= N; ++i) cin >> item_v[i];

    // Pre-sort orders
    vector<int> all_items(N);
    iota(all_items.begin(), all_items.end(), 1);

    order_v_minm = all_items;
    sort(order_v_minm.begin(), order_v_minm.end(), [](int a, int b) {
        if (item_v[a] != item_v[b]) return item_v[a] > item_v[b];
        if (item_m[a] != item_m[b]) return item_m[a] < item_m[b];
        return a < b;
    });

    order_v_maxm = all_items;
    sort(order_v_maxm.begin(), order_v_maxm.end(), [](int a, int b) {
        if (item_v[a] != item_v[b]) return item_v[a] > item_v[b];
        if (item_m[a] != item_m[b]) return item_m[a] > item_m[b];
        return a < b;
    });

    order_ratio_maxv = all_items;
    sort(order_ratio_maxv.begin(), order_ratio_maxv.end(), [](int a, int b) {
        double ra = (double)item_v[a] / item_m[a];
        double rb = (double)item_v[b] / item_m[b];
        if (abs(ra - rb) > 1e-9) return ra > rb;
        if (item_v[a] != item_v[b]) return item_v[a] > item_v[b];
        return a < b;
    });

    order_ratio_minm = all_items;
    sort(order_ratio_minm.begin(), order_ratio_minm.end(), [](int a, int b) {
        double ra = (double)item_v[a] / item_m[a];
        double rb = (double)item_v[b] / item_m[b];
        if (abs(ra - rb) > 1e-9) return ra > rb;
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

    vector<double> bot_w(BOT_COUNT, 1.0);
    vector<int> bot_matches(BOT_COUNT, 0);
    int total_obs = 0;

    Mask sim_avail = avail;
    vector<int> sim_M = M;

    auto observe_moves = [&](const vector<int>& round_moves) {
        int p1_m = round_moves[0];
        if (p1_m > 0 && ((sim_avail >> p1_m) & 1)) {
            sim_avail &= ~(ONE << p1_m);
            sim_M[0] += item_m[p1_m];
        }

        for (int d = 1; d < K; ++d) {
            int ad = round_moves[d];
            total_obs++;
            int max_other_d = 0;
            for (int j = 0; j < K; ++j) if (j != d) max_other_d = max(max_other_d, sim_M[j]);

            for (int b = 0; b < BOT_COUNT; ++b) {
                int pred = bot_pick((BotType)b, sim_avail, sim_M[d], max_other_d);
                if (pred == ad) {
                    bot_matches[b]++;
                }
            }

            if (ad > 0) {
                if ((sim_avail >> ad) & 1) sim_avail &= ~(ONE << ad);
                sim_M[d] += item_m[ad];
            }
        }

        if (total_obs > 0) {
            int max_match = *max_element(bot_matches.begin(), bot_matches.end());
            for (int b = 0; b < BOT_COUNT; ++b) {
                int diff = max_match - bot_matches[b];
                bot_w[b] = pow(0.25, diff);
            }
        }
    };

    auto choose_move = [&](int r) -> int {
        int cap0 = W - M[0];
        vector<int> valid;
        for (int i = 1; i <= N; ++i) {
            if (((avail >> i) & 1) && item_m[i] <= cap0) {
                valid.push_back(i);
            }
        }

        if (valid.empty()) return 0;
        if (valid.size() == 1) return valid[0];

        // Emergency fast pick if total execution time approaches 7.5 seconds
        auto now = chrono::steady_clock::now();
        double elapsed_sec = chrono::duration<double>(now - global_start_time).count();
        if (elapsed_sec > 7.5) {
            for (int id : order_ratio_maxv) {
                if (((avail >> id) & 1) && item_m[id] < cap0) return id;
            }
            return valid[0];
        }

        double sum_w = 0.0;
        for (int b = 0; b < BOT_COUNT; ++b) sum_w += bot_w[b];

        vector<pair<BotType, double>> active_bots;
        for (int b = 0; b < BOT_COUNT; ++b) {
            double nw = bot_w[b] / sum_w;
            if (nw > 0.01) active_bots.push_back({(BotType)b, nw});
        }
        if (active_bots.empty()) {
            int best_b = 0;
            for (int b = 1; b < BOT_COUNT; ++b) if (bot_w[b] > bot_w[best_b]) best_b = b;
            active_bots.push_back({(BotType)best_b, 1.0});
        }

        // Candidate filtering: select high-potential items (at most 16 items)
        vector<int> cands;
        vector<bool> seen(N + 1, false);

        int ratio_lim = min((int)valid.size(), 10);
        int ratio_cnt = 0;
        for (int id : order_ratio_maxv) {
            if (((avail >> id) & 1) && item_m[id] <= cap0) {
                if (!seen[id]) { seen[id] = true; cands.push_back(id); }
                if (++ratio_cnt >= ratio_lim) break;
            }
        }

        int v_lim = min((int)valid.size(), 4);
        int v_cnt = 0;
        for (int id : order_v_minm) {
            if (((avail >> id) & 1) && item_m[id] <= cap0) {
                if (!seen[id]) { seen[id] = true; cands.push_back(id); }
                if (++v_cnt >= v_lim) break;
            }
        }

        int m_lim = min((int)valid.size(), 2);
        int m_cnt = 0;
        for (int id : order_min_m) {
            if (((avail >> id) & 1) && item_m[id] <= cap0) {
                if (!seen[id]) { seen[id] = true; cands.push_back(id); }
                if (++m_cnt >= m_lim) break;
            }
        }

        struct CandEval {
            double surv_weight;
            double exp_val;
            double exp_margin;
            int cand;
        };

        vector<CandEval> evals;

        const P1Policy POLS[] = {
            P1Policy::DYNAMIC_TARGET_1,
            P1Policy::DYNAMIC_TARGET_2,
            P1Policy::DYNAMIC_TARGET_3,
            P1Policy::GREEDY_RATIO,
            P1Policy::STAY_LIGHT,
            P1Policy::MIN_M,
            P1Policy::GREEDY_V
        };

        for (int cand : cands) {
            if (M[0] + item_m[cand] == W) {
                evals.push_back({-1.0, -1e9, -(double)W, cand});
                continue;
            }

            double surv_w = 0.0;
            double exp_v = 0.0;
            double exp_m = 0.0;

            for (auto& ab : active_bots) {
                BotType btype = ab.first;
                double bw = ab.second;

                // Simulate candidate move and bots finishing round r
                Mask cand_avail = avail & ~(ONE << cand);
                vector<int> cand_M = M;
                vector<int> cand_V = V;
                cand_M[0] += item_m[cand];
                cand_V[0] += item_v[cand];

                for (int d = 1; d < K; ++d) {
                    int max_other = 0;
                    for (int j = 0; j < K; ++j) if (j != d) max_other = max(max_other, cand_M[j]);
                    int pick = bot_pick(btype, cand_avail, cand_M[d], max_other);
                    if (pick > 0) {
                        cand_avail &= ~(ONE << pick);
                        cand_M[d] += item_m[pick];
                        cand_V[d] += item_v[pick];
                    }
                }

                // Project heaviest bot final weight ONCE per (cand, btype)
                int projected_bot_max = project_bot_max(cand_avail, cand_M, btype, r + 1);

                bool best_surv = false;
                int best_sc = -1;
                int best_margin = -1e9;

                for (P1Policy pol : POLS) {
                    RolloutResult res = simulate_rollout(
                        cand_avail, cand_M, cand_V,
                        r + 1, projected_bot_max, btype, pol
                    );
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

        sort(evals.begin(), evals.end(), [](const CandEval& a, const CandEval& b) {
            if (abs(a.surv_weight - b.surv_weight) > 1e-6) return a.surv_weight > b.surv_weight;
            if (abs(a.exp_val - b.exp_val) > 1e-4) return a.exp_val > b.exp_val;
            return a.exp_margin > b.exp_margin;
        });

        if (evals[0].surv_weight <= 0) {
            sort(evals.begin(), evals.end(), [](const CandEval& a, const CandEval& b) {
                if (abs(a.exp_margin - b.exp_margin) > 1e-4) return a.exp_margin > b.exp_margin;
                double ra = (double)item_v[a.cand] / item_m[a.cand];
                double rb = (double)item_v[b.cand] / item_m[b.cand];
                return ra > rb;
            });
        }

        return evals[0].cand;
    };

    for (int r = 1; r <= T; ++r) {
        int a1 = choose_move(r);
        cout << a1 << endl; // flushes stdout immediately

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
                if ((avail >> ad) & 1) avail &= ~(ONE << ad);
                M[d] += item_m[ad];
                V[d] += item_v[ad];
            }
        }

        vector<int> round_moves = {a1};
        round_moves.insert(round_moves.end(), bot_moves.begin(), bot_moves.end());
        observe_moves(round_moves);
    }

    return 0;
}
