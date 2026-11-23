#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>
#include <chrono>
#include <cmath>
#include <numeric>
#include <tuple>

using namespace std;

// 128-bit mask for up to 100 items (N <= 100)
using Mask = unsigned __int128;
static const Mask ONE = 1;

int K, N, T, W;
vector<int> item_m;
vector<int> item_v;

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
    KNAPSACK_W,
    KNAPSACK_W1,
    KNAPSACK_W2,
    NUM_BOTS
};

const int BOT_COUNT = (int)BotType::NUM_BOTS;

// 0/1 Knapsack helper with bitmask for small capacities (W <= 50)
int solve_knapsack_mask(Mask avail, int cap, Mask& best_mask) {
    if (cap <= 0) {
        best_mask = 0;
        return 0;
    }
    vector<int> dp(cap + 1, 0);
    vector<Mask> mask(cap + 1, 0);

    for (int i = 1; i <= N; ++i) {
        if (((avail >> i) & 1) && item_m[i] <= cap) {
            int wi = item_m[i], vi = item_v[i];
            Mask bit = (ONE << i);
            for (int w = cap; w >= wi; --w) {
                int nv = dp[w - wi] + vi;
                if (nv > dp[w]) {
                    dp[w] = nv;
                    mask[w] = mask[w - wi] | bit;
                }
            }
        }
    }

    int best_v = 0;
    best_mask = 0;
    for (int w = 1; w <= cap; ++w) {
        if (dp[w] > best_v) {
            best_v = dp[w];
            best_mask = mask[w];
        }
    }
    return best_v;
}

int bot_pick(BotType type, Mask avail, int cur_m, int max_other) {
    int cap = W - cur_m;
    if (cap <= 0) return 0;

    int best_id = 0;

    switch (type) {
        case BotType::GREEDY_V_MINM: {
            int best_v = -1, best_m = 1e9;
            for (int i = 1; i <= N; ++i) {
                if (((avail >> i) & 1) && item_m[i] <= cap) {
                    if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                        best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                    }
                }
            }
            return best_id;
        }
        case BotType::GREEDY_V_MAXM: {
            int best_v = -1, best_m = -1;
            for (int i = 1; i <= N; ++i) {
                if (((avail >> i) & 1) && item_m[i] <= cap) {
                    if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] > best_m || (item_m[i] == best_m && i < best_id)))) {
                        best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                    }
                }
            }
            return best_id;
        }
        case BotType::GREEDY_RATIO_MAXV: {
            double best_r = -1.0; int best_v = -1;
            for (int i = 1; i <= N; ++i) {
                if (((avail >> i) & 1) && item_m[i] <= cap) {
                    double r = (double)item_v[i] / item_m[i];
                    if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < best_id)))) {
                        best_r = r; best_v = item_v[i]; best_id = i;
                    }
                }
            }
            return best_id;
        }
        case BotType::GREEDY_RATIO_MINM: {
            double best_r = -1.0; int best_m = 1e9;
            for (int i = 1; i <= N; ++i) {
                if (((avail >> i) & 1) && item_m[i] <= cap) {
                    double r = (double)item_v[i] / item_m[i];
                    if (r > best_r || (abs(r - best_r) < 1e-9 && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                        best_r = r; best_m = item_m[i]; best_id = i;
                    }
                }
            }
            return best_id;
        }
        case BotType::SAFE_W1_RATIO: {
            int safe_cap = min(cap, W - 1 - cur_m);
            if (safe_cap > 0) {
                double best_r = -1.0; int best_v = -1;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        double r = (double)item_v[i] / item_m[i];
                        if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < best_id)))) {
                            best_r = r; best_v = item_v[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::GREEDY_RATIO_MAXV, avail, cur_m, max_other);
        }
        case BotType::SAFE_W1_V: {
            int safe_cap = min(cap, W - 1 - cur_m);
            if (safe_cap > 0) {
                int best_v = -1, best_m = 1e9;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                            best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::GREEDY_V_MINM, avail, cur_m, max_other);
        }
        case BotType::SAFE_W2_RATIO: {
            int safe_cap = min(cap, max(1, W - 2) - cur_m);
            if (safe_cap > 0) {
                double best_r = -1.0; int best_v = -1;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        double r = (double)item_v[i] / item_m[i];
                        if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < best_id)))) {
                            best_r = r; best_v = item_v[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::SAFE_W1_RATIO, avail, cur_m, max_other);
        }
        case BotType::SAFE_W2_V: {
            int safe_cap = min(cap, max(1, W - 2) - cur_m);
            if (safe_cap > 0) {
                int best_v = -1, best_m = 1e9;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                            best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::SAFE_W1_V, avail, cur_m, max_other);
        }
        case BotType::SAFE_W3_RATIO: {
            int safe_cap = min(cap, max(1, W - 3) - cur_m);
            if (safe_cap > 0) {
                double best_r = -1.0; int best_v = -1;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        double r = (double)item_v[i] / item_m[i];
                        if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < best_id)))) {
                            best_r = r; best_v = item_v[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::SAFE_W2_RATIO, avail, cur_m, max_other);
        }
        case BotType::SAFE_W3_V: {
            int safe_cap = min(cap, max(1, W - 3) - cur_m);
            if (safe_cap > 0) {
                int best_v = -1, best_m = 1e9;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                            best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::SAFE_W2_V, avail, cur_m, max_other);
        }
        case BotType::BEAT_MAX_V: {
            int limit = (max_other > 0) ? max_other : W;
            int safe_cap = min(cap, limit - 1 - cur_m);
            if (safe_cap > 0) {
                int best_v = -1, best_m = 1e9;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                            best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::SAFE_W1_V, avail, cur_m, max_other);
        }
        case BotType::BEAT_MAX_RATIO: {
            int limit = (max_other > 0) ? max_other : W;
            int safe_cap = min(cap, limit - 1 - cur_m);
            if (safe_cap > 0) {
                double best_r = -1.0; int best_v = -1;
                for (int i = 1; i <= N; ++i) {
                    if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                        double r = (double)item_v[i] / item_m[i];
                        if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < best_id)))) {
                            best_r = r; best_v = item_v[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::SAFE_W1_RATIO, avail, cur_m, max_other);
        }
        case BotType::MIN_M: {
            int best_m = 1e9, best_v = -1;
            for (int i = 1; i <= N; ++i) {
                if (((avail >> i) & 1) && item_m[i] <= cap) {
                    if (item_m[i] < best_m || (item_m[i] == best_m && (item_v[i] > best_v || (item_v[i] == best_v && i < best_id)))) {
                        best_m = item_m[i]; best_v = item_v[i]; best_id = i;
                    }
                }
            }
            return best_id;
        }
        case BotType::KNAPSACK_W: {
            Mask bmask = 0;
            solve_knapsack_mask(avail, cap, bmask);
            if (bmask > 0) {
                int best_v = -1, best_m = 1e9;
                for (int i = 1; i <= N; ++i) {
                    if ((bmask >> i) & 1) {
                        if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                            best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                        }
                    }
                }
                if (best_id > 0) return best_id;
            }
            return bot_pick(BotType::GREEDY_V_MINM, avail, cur_m, max_other);
        }
        case BotType::KNAPSACK_W1: {
            int safe_cap = min(cap, W - 1 - cur_m);
            if (safe_cap > 0) {
                Mask bmask = 0;
                solve_knapsack_mask(avail, safe_cap, bmask);
                if (bmask > 0) {
                    int best_v = -1, best_m = 1e9;
                    for (int i = 1; i <= N; ++i) {
                        if ((bmask >> i) & 1) {
                            if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                                best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                            }
                        }
                    }
                    if (best_id > 0) return best_id;
                }
            }
            return bot_pick(BotType::SAFE_W1_V, avail, cur_m, max_other);
        }
        case BotType::KNAPSACK_W2: {
            int safe_cap = min(cap, max(1, W - 2) - cur_m);
            if (safe_cap > 0) {
                Mask bmask = 0;
                solve_knapsack_mask(avail, safe_cap, bmask);
                if (bmask > 0) {
                    int best_v = -1, best_m = 1e9;
                    for (int i = 1; i <= N; ++i) {
                        if ((bmask >> i) & 1) {
                            if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < best_id)))) {
                                best_v = item_v[i]; best_m = item_m[i]; best_id = i;
                            }
                        }
                    }
                    if (best_id > 0) return best_id;
                }
            }
            return bot_pick(BotType::SAFE_W2_V, avail, cur_m, max_other);
        }
        default:
            return 0;
    }
}

struct RolloutResult {
    bool survived;
    int score;
    int margin;
};

enum class P1Policy {
    GREEDY_RATIO,
    TARGET_W1,
    TARGET_W2,
    TARGET_W3,
    STAY_LIGHT,
    MIN_M,
    GREEDY_V,
    NUM_POLICIES
};

RolloutResult simulate_rollout(
    int cand, int start_round, Mask cur_avail,
    const vector<int>& init_M, const vector<int>& init_V,
    BotType btype, P1Policy pol
) {
    Mask avail = cur_avail;
    vector<int> M = init_M;
    vector<int> V = init_V;

    // P1 start_round move
    if (cand > 0) {
        avail &= ~(ONE << cand);
        M[0] += item_m[cand];
        V[0] += item_v[cand];
    }

    // Bots complete start_round
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

    int target_W = W - 1;
    if (pol == P1Policy::TARGET_W2) target_W = max(1, W - 2);
    else if (pol == P1Policy::TARGET_W3) target_W = max(1, W - 3);

    for (int r = start_round + 1; r <= T; ++r) {
        if (!avail) break;

        int cap0 = W - M[0];
        int fut_a = 0;

        if (cap0 > 0) {
            switch (pol) {
                case P1Policy::GREEDY_RATIO: {
                    int safe_cap = min(cap0, W - 1 - M[0]);
                    double best_r = -1.0; int best_v = -1;
                    if (safe_cap > 0) {
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                                double r = (double)item_v[i] / item_m[i];
                                if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                    best_r = r; best_v = item_v[i]; fut_a = i;
                                }
                            }
                        }
                    }
                    if (fut_a == 0) {
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= cap0) {
                                double r = (double)item_v[i] / item_m[i];
                                if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                    best_r = r; best_v = item_v[i]; fut_a = i;
                                }
                            }
                        }
                    }
                    break;
                }
                case P1Policy::TARGET_W1:
                case P1Policy::TARGET_W2:
                case P1Policy::TARGET_W3: {
                    int safe_cap = min(cap0, target_W - M[0]);
                    double best_r = -1.0; int best_v = -1;
                    if (safe_cap > 0) {
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                                double r = (double)item_v[i] / item_m[i];
                                if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                    best_r = r; best_v = item_v[i]; fut_a = i;
                                }
                            }
                        }
                    }
                    if (fut_a == 0) {
                        int safe_w = min(cap0, W - 1 - M[0]);
                        if (safe_w > 0) {
                            for (int i = 1; i <= N; ++i) {
                                if (((avail >> i) & 1) && item_m[i] <= safe_w) {
                                    double r = (double)item_v[i] / item_m[i];
                                    if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                        best_r = r; best_v = item_v[i]; fut_a = i;
                                    }
                                }
                            }
                        }
                    }
                    if (fut_a == 0) {
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= cap0) {
                                double r = (double)item_v[i] / item_m[i];
                                if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                    best_r = r; best_v = item_v[i]; fut_a = i;
                                }
                            }
                        }
                    }
                    break;
                }
                case P1Policy::STAY_LIGHT: {
                    int max_b = 0;
                    for (int j = 1; j < K; ++j) max_b = max(max_b, M[j]);
                    int safe_cap = min(cap0, max_b - M[0]);
                    double best_r = -1.0; int best_v = -1;
                    if (safe_cap > 0) {
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                                double r = (double)item_v[i] / item_m[i];
                                if (r > best_r || (abs(r - best_r) < 1e-9 && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                    best_r = r; best_v = item_v[i]; fut_a = i;
                                }
                            }
                        }
                    }
                    if (fut_a == 0) {
                        int best_m = 1e9;
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= cap0) {
                                if (item_m[i] < best_m || (item_m[i] == best_m && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                    best_m = item_m[i]; best_v = item_v[i]; fut_a = i;
                                }
                            }
                        }
                    }
                    break;
                }
                case P1Policy::MIN_M: {
                    int best_m = 1e9, best_v = -1;
                    for (int i = 1; i <= N; ++i) {
                        if (((avail >> i) & 1) && item_m[i] <= cap0) {
                            if (item_m[i] < best_m || (item_m[i] == best_m && (item_v[i] > best_v || (item_v[i] == best_v && i < fut_a)))) {
                                best_m = item_m[i]; best_v = item_v[i]; fut_a = i;
                            }
                        }
                    }
                    break;
                }
                case P1Policy::GREEDY_V: {
                    int safe_cap = min(cap0, W - 1 - M[0]);
                    int best_v = -1, best_m = 1e9;
                    if (safe_cap > 0) {
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= safe_cap) {
                                if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < fut_a)))) {
                                    best_v = item_v[i]; best_m = item_m[i]; fut_a = i;
                                }
                            }
                        }
                    }
                    if (fut_a == 0) {
                        for (int i = 1; i <= N; ++i) {
                            if (((avail >> i) & 1) && item_m[i] <= cap0) {
                                if (item_v[i] > best_v || (item_v[i] == best_v && (item_m[i] < best_m || (item_m[i] == best_m && i < fut_a)))) {
                                    best_v = item_v[i]; best_m = item_m[i]; fut_a = i;
                                }
                            }
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

// Lookahead Beam Search for Player 1 from round start_round+1 to T
RolloutResult beam_search_lookahead(
    int cand, int start_round, Mask cur_avail,
    const vector<int>& init_M, const vector<int>& init_V,
    BotType btype, int beam_width = 16
) {
    struct BeamNode {
        Mask avail;
        vector<int> M;
        vector<int> V;
    };

    Mask avail = cur_avail;
    vector<int> M = init_M;
    vector<int> V = init_V;

    if (cand > 0) {
        avail &= ~(ONE << cand);
        M[0] += item_m[cand];
        V[0] += item_v[cand];
    }

    // bots finish start_round
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

    vector<BeamNode> beam = { {avail, M, V} };

    for (int r = start_round + 1; r <= T; ++r) {
        vector<BeamNode> next_beam;

        for (const auto& node : beam) {
            int cap0 = W - node.M[0];
            vector<int> moves;

            if (cap0 <= 0) {
                moves.push_back(0);
            } else {
                // Find potential moves for P1
                // 1) Top by ratio
                // 2) Top by value
                // 3) Min mass
                vector<int> valid;
                for (int i = 1; i <= N; ++i) {
                    if (((node.avail >> i) & 1) && item_m[i] <= cap0) {
                        valid.push_back(i);
                    }
                }

                if (valid.empty()) {
                    moves.push_back(0);
                } else {
                    sort(valid.begin(), valid.end(), [](int a, int b) {
                        double ra = (double)item_v[a] / item_m[a];
                        double rb = (double)item_v[b] / item_m[b];
                        if (abs(ra - rb) > 1e-9) return ra > rb;
                        return item_v[a] > item_v[b];
                    });
                    int take_cnt = min((int)valid.size(), 4);
                    for (int k = 0; k < take_cnt; ++k) moves.push_back(valid[k]);

                    // Add top by raw value
                    sort(valid.begin(), valid.end(), [](int a, int b) {
                        if (item_v[a] != item_v[b]) return item_v[a] > item_v[b];
                        return item_m[a] < item_m[b];
                    });
                    if (find(moves.begin(), moves.end(), valid[0]) == moves.end()) {
                        moves.push_back(valid[0]);
                    }

                    // Add min mass
                    sort(valid.begin(), valid.end(), [](int a, int b) {
                        if (item_m[a] != item_m[b]) return item_m[a] < item_m[b];
                        return item_v[a] > item_v[b];
                    });
                    if (find(moves.begin(), moves.end(), valid[0]) == moves.end()) {
                        moves.push_back(valid[0]);
                    }
                }
            }

            for (int act : moves) {
                BeamNode succ = node;
                if (act > 0) {
                    succ.avail &= ~(ONE << act);
                    succ.M[0] += item_m[act];
                    succ.V[0] += item_v[act];
                }

                // bots turn
                for (int d = 1; d < K; ++d) {
                    int max_other = 0;
                    for (int j = 0; j < K; ++j) if (j != d) max_other = max(max_other, succ.M[j]);
                    int pick = bot_pick(btype, succ.avail, succ.M[d], max_other);
                    if (pick > 0) {
                        succ.avail &= ~(ONE << pick);
                        succ.M[d] += item_m[pick];
                        succ.V[d] += item_v[pick];
                    }
                }

                next_beam.push_back(succ);
            }
        }

        // Rank states in next_beam
        sort(next_beam.begin(), next_beam.end(), [](const BeamNode& a, const BeamNode& b) {
            int max_a = 0, max_b = 0;
            for (int j = 1; j < K; ++j) max_a = max(max_a, a.M[j]);
            for (int j = 1; j < K; ++j) max_b = max(max_b, b.M[j]);

            bool surv_a = (a.M[0] < max_a && a.M[0] < W);
            bool surv_b = (b.M[0] < max_b && b.M[0] < W);

            if (surv_a != surv_b) return surv_a > surv_b;
            if (a.V[0] != b.V[0]) return a.V[0] > b.V[0];
            return (max_a - a.M[0]) > (max_b - b.M[0]);
        });

        if ((int)next_beam.size() > beam_width) {
            next_beam.resize(beam_width);
        }
        beam = std::move(next_beam);
    }

    // Evaluate best node from final beam
    bool best_surv = false;
    int best_score = -1;
    int best_margin = -1e9;

    for (const auto& node : beam) {
        int max_other = 0;
        for (int j = 1; j < K; ++j) max_other = max(max_other, node.M[j]);
        bool surv = (node.M[0] < max_other && node.M[0] < W);
        int sc = surv ? node.V[0] : 0;
        int mg = max_other - node.M[0];

        if (make_tuple(surv, sc, mg) > make_tuple(best_surv, best_score, best_margin)) {
            best_surv = surv;
            best_score = sc;
            best_margin = mg;
        }
    }

    return {best_surv, best_score, best_margin};
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> K >> N >> T >> W)) return 0;

    item_m.assign(N + 1, 0);
    item_v.assign(N + 1, 0);

    for (int i = 1; i <= N; ++i) cin >> item_m[i];
    for (int i = 1; i <= N; ++i) cin >> item_v[i];

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

        struct CandEval {
            double surv_weight;
            double exp_val;
            double exp_margin;
            int cand;
        };

        vector<CandEval> evals;

        const P1Policy POLS[] = {
            P1Policy::GREEDY_RATIO,
            P1Policy::TARGET_W1,
            P1Policy::TARGET_W2,
            P1Policy::TARGET_W3,
            P1Policy::STAY_LIGHT,
            P1Policy::MIN_M,
            P1Policy::GREEDY_V
        };

        for (int cand : valid) {
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

                bool best_surv = false;
                int best_sc = -1;
                int best_margin = -1e9;

                // 1. Static policy rollouts
                for (P1Policy pol : POLS) {
                    RolloutResult res = simulate_rollout(cand, r, avail, M, V, btype, pol);
                    if (make_tuple(res.survived, res.score, res.margin) >
                        make_tuple(best_surv, best_sc, best_margin)) {
                        best_surv = res.survived;
                        best_sc = res.score;
                        best_margin = res.margin;
                    }
                }

                // 2. Beam search lookahead
                RolloutResult bres = beam_search_lookahead(cand, r, avail, M, V, btype, 12);
                if (make_tuple(bres.survived, bres.score, bres.margin) >
                    make_tuple(best_surv, best_sc, best_margin)) {
                    best_surv = bres.survived;
                    best_sc = bres.score;
                    best_margin = bres.margin;
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
        cout << a1 << endl; // endl flushes stdout immediately

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
