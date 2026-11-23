#define _GNU_SOURCE
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

static constexpr int MAX_N = 64;
static constexpr int MAX_D = 8;
static constexpr int MAX_T = 128;

inline void my_sincos(double theta, double& s, double& c) {
#if defined(__APPLE__)
    __sincos(theta, &s, &c);
#elif defined(__GNUC__) || defined(__GNUG__)
    __builtin_sincos(theta, &s, &c);
#else
    s = sin(theta);
    c = cos(theta);
#endif
}

struct Term {
    double a, nu, delta, b, phi;
    double two_a_nu;
    double v[MAX_D];
};

struct Func {
    double lambda;
    int M;
    double s[MAX_D];
    vector<Term> terms;
    double f0;
    double inv_D;
};

struct State {
    double x[MAX_N][MAX_D];
    double g[MAX_N][MAX_D];
    double best_f[MAX_N];
    uint8_t alive[MAX_N];
};

struct Problem {
    int N, d, T;
    double alpha_min, alpha_max;
    double log_min, log_max;
    Func funcs[MAX_N];
    State init_state;

    void eval_point_and_grad(int i, const double* x, double& f_val, double* g_out, bool need_grad) const {
        const Func& fn = funcs[i];
        double norm2 = 0.0;
        for (int k = 0; k < d; ++k) {
            norm2 += x[k] * x[k];
        }
        f_val = 0.5 * fn.lambda * norm2;

        if (need_grad) {
            for (int k = 0; k < d; ++k) {
                g_out[k] = fn.lambda * x[k];
            }
        }

        for (int r = 0; r < fn.M; ++r) {
            const Term& tr = fn.terms[r];
            double z = -tr.b;
            for (int k = 0; k < d; ++k) {
                z += tr.v[k] * x[k];
            }
            double h = tr.delta + z * z;
            double inv_h = 1.0 / h;
            double theta = tr.nu * inv_h + tr.phi;
            double s_val, c_val;
            my_sincos(theta, s_val, c_val);
            f_val += tr.a * s_val;

            if (need_grad) {
                double coeff = (tr.two_a_nu * z * c_val) * (inv_h * inv_h);
                for (int k = 0; k < d; ++k) {
                    g_out[k] -= coeff * tr.v[k];
                }
            }
        }
    }

    void init() {
        log_min = log(alpha_min);
        log_max = log(alpha_max);
        for (int i = 0; i < N; ++i) {
            Func& fn = funcs[i];
            double L = 0.0;
            for (int r = 0; r < fn.M; ++r) {
                L -= fn.terms[r].a;
                fn.terms[r].two_a_nu = 2.0 * fn.terms[r].a * fn.terms[r].nu;
            }
            eval_point_and_grad(i, fn.s, fn.f0, init_state.g[i], true);
            fn.inv_D = 1.0 / (fn.f0 - L);
            for (int k = 0; k < d; ++k) {
                init_state.x[i][k] = fn.s[k];
            }
            init_state.best_f[i] = fn.f0;
            init_state.alive[i] = 1;
        }
    }

    inline void step_state(const State& cur, double alpha, State& nxt, bool need_grad) const {
        for (int i = 0; i < N; ++i) {
            if (!cur.alive[i]) {
                nxt.alive[i] = 0;
                nxt.best_f[i] = cur.best_f[i];
                continue;
            }
            bool ok = true;
            for (int k = 0; k < d; ++k) {
                double val = cur.x[i][k] - alpha * cur.g[i][k];
                if (!isfinite(val) || val < -10.0 || val > 10.0) {
                    ok = false;
                    break;
                }
                nxt.x[i][k] = val;
            }
            if (!ok) {
                nxt.alive[i] = 0;
                nxt.best_f[i] = cur.best_f[i];
                continue;
            }
            nxt.alive[i] = 1;
            double f_val;
            eval_point_and_grad(i, nxt.x[i], f_val, nxt.g[i], need_grad);
            nxt.best_f[i] = (f_val < cur.best_f[i]) ? f_val : cur.best_f[i];
        }
    }

    inline double score_state(const State& st) const {
        double sum = 0.0;
        for (int i = 0; i < N; ++i) {
            double q = (funcs[i].f0 - st.best_f[i]) * funcs[i].inv_D;
            if (q > 1.0) q = 1.0;
            else if (q < 0.0) q = 0.0;
            sum += q;
        }
        return sum / N;
    }

    double eval_full(const vector<double>& alpha) const {
        State buf[2];
        buf[0] = init_state;
        int cur = 0;
        for (int t = 0; t < T; ++t) {
            step_state(buf[cur], alpha[t], buf[cur ^ 1], t + 1 < T);
            cur ^= 1;
        }
        return score_state(buf[cur]);
    }

    double eval_suffix(const vector<double>& alpha, int start_t, const vector<State>& cache) const {
        State buf[2];
        const State* src = &cache[start_t];
        int cur = 0;
        for (int t = start_t; t < T; ++t) {
            step_state(*src, alpha[t], buf[cur], t + 1 < T);
            src = &buf[cur];
            cur ^= 1;
        }
        return score_state(*src);
    }

    double update_cache(const vector<double>& alpha, int start_t, vector<State>& cache) const {
        for (int t = start_t; t < T; ++t) {
            step_state(cache[t], alpha[t], cache[t + 1], t + 1 < T);
        }
        return score_state(cache[T]);
    }

    inline double clamp_alpha(double a) const {
        if (a < alpha_min) return alpha_min;
        if (a > alpha_max) return alpha_max;
        return a;
    }

    inline double log_lerp(double u) const {
        return clamp_alpha(exp(log_min + u * (log_max - log_min)));
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    auto t_start = chrono::steady_clock::now();
    auto elapsed = [&]() -> double {
        return chrono::duration<double>(chrono::steady_clock::now() - t_start).count();
    };

    Problem prob;
    if (!(cin >> prob.N >> prob.d >> prob.T)) return 0;
    cin >> prob.alpha_min >> prob.alpha_max;

    for (int i = 0; i < prob.N; ++i) {
        int w_unused;
        Func& fn = prob.funcs[i];
        cin >> w_unused >> fn.lambda >> fn.M;
        for (int k = 0; k < prob.d; ++k) {
            cin >> fn.s[k];
        }
        fn.terms.resize(fn.M);
        for (int r = 0; r < fn.M; ++r) {
            Term& tr = fn.terms[r];
            cin >> tr.a >> tr.nu >> tr.delta >> tr.b >> tr.phi;
            for (int k = 0; k < prob.d; ++k) {
                cin >> tr.v[k];
            }
        }
    }

    prob.init();

    // Strict wall-clock safety: total limit 5.0s, internal cap 4.65s
    const double TIME_LIMIT = 4.65;
    const double SA_END = 4.10;

    mt19937_64 rng(20260417ULL);
    uniform_real_distribution<double> u01(0.0, 1.0);

    vector<double> best_alpha(prob.T, prob.log_lerp(0.5));
    double best_score = prob.eval_full(best_alpha);

    auto consider = [&](const vector<double>& cand) {
        double sc = prob.eval_full(cand);
        if (sc > best_score) {
            best_score = sc;
            best_alpha = cand;
        }
        return sc;
    };

    // =========================================================================
    // СТАДИЯ 1: Портфель параметрических расписаний (до 0.4 с)
    // =========================================================================
    {
        vector<double> cand(prob.T);

        // 1.1 Константные расписания
        int num_const = 30;
        for (int i = 0; i < num_const; ++i) {
            double a = prob.log_lerp(double(i) / (num_const - 1));
            fill(cand.begin(), cand.end(), a);
            consider(cand);
        }

        // 1.2 Экспоненциальные рампы
        vector<double> ends = {0.0, 0.15, 0.3, 0.45, 0.6, 0.75, 0.9, 1.0};
        for (double u0 : ends) {
            for (double u1 : ends) {
                if (u0 == u1) continue;
                for (int t = 0; t < prob.T; ++t) {
                    double frac = (prob.T > 1) ? double(t) / (prob.T - 1) : 0.0;
                    cand[t] = prob.log_lerp(u0 + (u1 - u0) * frac);
                }
                consider(cand);
                if (elapsed() > 0.4) break;
            }
            if (elapsed() > 0.4) break;
        }

        // 1.3 1cycle политики (Warmup + Cosine Decay)
        if (elapsed() < 0.4 && prob.T >= 2) {
            for (double peak_u : {0.5, 0.75, 0.95}) {
                for (double wp : {0.2, 0.4}) {
                    int cut = max(1, min(prob.T - 1, int(round(wp * prob.T))));
                    for (int t = 0; t < prob.T; ++t) {
                        if (t < cut) {
                            cand[t] = prob.log_lerp(double(t) / cut * peak_u);
                        } else {
                            double frac = (prob.T - 1 > cut) ? double(t - cut) / (prob.T - 1 - cut) : 1.0;
                            double cos_val = 0.5 * (1.0 + cos(M_PI * frac));
                            cand[t] = prob.log_lerp(cos_val * peak_u);
                        }
                    }
                    consider(cand);
                }
            }
        }

        // 1.4 Step Decay (ступенчатое падение)
        if (elapsed() < 0.4 && prob.T >= 3) {
            for (double hi_u : {0.6, 0.8, 1.0}) {
                for (double decay_factor : {0.2, 0.35, 0.5}) {
                    double a_hi = prob.log_lerp(hi_u);
                    double a_mid = prob.clamp_alpha(a_hi * decay_factor);
                    double a_lo = prob.clamp_alpha(a_mid * decay_factor);
                    int cut1 = int(0.4 * prob.T);
                    int cut2 = int(0.75 * prob.T);
                    for (int t = 0; t < prob.T; ++t) {
                        if (t < cut1) cand[t] = a_hi;
                        else if (t < cut2) cand[t] = a_mid;
                        else cand[t] = a_lo;
                    }
                    consider(cand);
                }
            }
        }
    }

    // =========================================================================
    // СТАДИЯ 2: Beam Search (до 1.0 с)
    // =========================================================================
    if (elapsed() < 0.8) {
        struct BeamNode {
            State st;
            vector<double> path;
            double sc;
            int last_bucket;
        };

        auto t_bench0 = chrono::steady_clock::now();
        prob.eval_full(best_alpha);
        double single_full_sec = max(1e-6, chrono::duration<double>(chrono::steady_clock::now() - t_bench0).count());

        int num_grid = 11;
        int beam_width = 8;
        double est_beam_time = single_full_sec * beam_width * num_grid;
        if (est_beam_time > 0.6) {
            beam_width = max(2, int(0.5 / (single_full_sec * num_grid)));
        }

        vector<double> grid(num_grid);
        for (int g = 0; g < num_grid; ++g) {
            grid[g] = prob.log_lerp(double(g) / (num_grid - 1));
        }

        vector<BeamNode> beam(1);
        beam[0].st = prob.init_state;
        beam[0].path.reserve(prob.T);
        beam[0].sc = 0.0;
        beam[0].last_bucket = -1;

        for (int t = 0; t < prob.T && elapsed() < 1.0; ++t) {
            vector<BeamNode> next_beam;
            next_beam.reserve(beam.size() * num_grid);

            for (const auto& node : beam) {
                for (int g = 0; g < num_grid; ++g) {
                    BeamNode child;
                    prob.step_state(node.st, grid[g], child.st, t + 1 < prob.T);
                    child.sc = prob.score_state(child.st);
                    child.path = node.path;
                    child.path.push_back(grid[g]);
                    child.last_bucket = g;
                    next_beam.push_back(std::move(child));
                }
            }

            sort(next_beam.begin(), next_beam.end(), [](const BeamNode& a, const BeamNode& b) {
                return a.sc > b.sc;
            });

            vector<BeamNode> pruned;
            pruned.reserve(beam_width);
            vector<int> bucket_cnt(num_grid, 0);

            for (auto& cand : next_beam) {
                if ((int)pruned.size() >= beam_width) break;
                if (bucket_cnt[cand.last_bucket] < 2 || (int)pruned.size() < beam_width / 2) {
                    bucket_cnt[cand.last_bucket]++;
                    pruned.push_back(std::move(cand));
                }
            }
            if (pruned.empty() && !next_beam.empty()) {
                pruned.push_back(std::move(next_beam[0]));
            }
            beam = std::move(pruned);
        }

        for (const auto& node : beam) {
            if ((int)node.path.size() == prob.T && node.sc > best_score) {
                best_score = node.sc;
                best_alpha = node.path;
            }
        }
    }

    // =========================================================================
    // СТАДИЯ 3: Имитация отжига (SA) с глобальным масштабированием и адаптивным рестартом
    // =========================================================================
    vector<State> cache(prob.T + 1);
    cache[0] = prob.init_state;

    vector<double> cur_alpha = best_alpha;
    double cur_score = prob.update_cache(cur_alpha, 0, cache);

    double sa_start_time = elapsed();
    double sa_duration = max(0.1, SA_END - sa_start_time);

    const double T0 = 0.008;
    const double T1 = 1e-5;
    int iters_since_best = 0;

    vector<double> cand_alpha(prob.T);

    while (elapsed() < SA_END) {
        double progress = min(1.0, (elapsed() - sa_start_time) / sa_duration);
        double temp = T0 * pow(T1 / T0, progress);

        cand_alpha = cur_alpha;
        int start_t = 0;

        double r_move = u01(rng);
        if (r_move < 0.06) {
            // МУТАЦИЯ 0: Глобальное масштабирование ВСЕХ шагов
            double delta = (u01(rng) * 2.0 - 1.0) * (0.35 * (1.0 - 0.5 * progress));
            double factor = exp(delta);
            for (int t = 0; t < prob.T; ++t) {
                cand_alpha[t] = prob.clamp_alpha(cand_alpha[t] * factor);
            }
            start_t = 0;
        } else if (r_move < 0.48) {
            // МУТАЦИЯ 1: Локальное лог-возмущение одного шага
            int idx = rng() % prob.T;
            double sigma = (u01(rng) < 0.5) ? 0.25 : 1.0;
            double delta = (u01(rng) * 2.0 - 1.0) * sigma;
            cand_alpha[idx] = prob.clamp_alpha(cand_alpha[idx] * exp(delta));
            start_t = idx;
        } else if (r_move < 0.65) {
            // МУТАЦИЯ 2: Глобальный прыжок одного шага
            int idx = rng() % prob.T;
            cand_alpha[idx] = prob.log_lerp(u01(rng));
            start_t = idx;
        } else if (r_move < 0.85) {
            // МУТАЦИЯ 3: Масштабирование отрезка [L, R]
            int L = rng() % prob.T;
            int R = rng() % prob.T;
            if (L > R) swap(L, R);
            double delta = (u01(rng) * 2.0 - 1.0) * (0.6 * (1.0 - 0.5 * progress));
            double factor = exp(delta);
            for (int t = L; t <= R; ++t) {
                cand_alpha[t] = prob.clamp_alpha(cand_alpha[t] * factor);
            }
            start_t = L;
        } else if (r_move < 0.93 && prob.T >= 2) {
            // МУТАЦИЯ 4: Перестановка двух шагов
            int i = rng() % prob.T;
            int j = rng() % prob.T;
            if (i == j) j = (i + 1) % prob.T;
            swap(cand_alpha[i], cand_alpha[j]);
            start_t = min(i, j);
        } else if (prob.T >= 2) {
            // МУТАЦИЯ 5: Вставка и сдвиг
            int pos = rng() % (prob.T - 1);
            for (int t = prob.T - 1; t > pos; --t) {
                cand_alpha[t] = cand_alpha[t - 1];
            }
            cand_alpha[pos] = prob.log_lerp(u01(rng));
            start_t = pos;
        }

        double cand_score = prob.eval_suffix(cand_alpha, start_t, cache);
        double diff = cand_score - cur_score;

        if (diff >= 0.0 || u01(rng) < exp(diff / temp)) {
            cur_alpha = cand_alpha;
            cur_score = prob.update_cache(cur_alpha, start_t, cache);
            if (cur_score > best_score) {
                best_score = cur_score;
                best_alpha = cur_alpha;
                iters_since_best = 0;
            } else {
                iters_since_best++;
            }
        } else {
            iters_since_best++;
        }

        // Если долго нет улучшений — возвращаемся к глобальному лучшему
        if (iters_since_best > 400) {
            cur_alpha = best_alpha;
            cur_score = prob.update_cache(cur_alpha, 0, cache);
            iters_since_best = 0;
        }
    }

    // =========================================================================
    // СТАДИЯ 4: Покоординатная шлифовка (до 4.60 с)
    // =========================================================================
    cur_alpha = best_alpha;
    cur_score = prob.update_cache(cur_alpha, 0, cache);

    vector<double> deltas = {0.4, 0.15, 0.05, 0.015, 0.004};
    for (double delta : deltas) {
        if (elapsed() >= TIME_LIMIT) break;
        bool improved = true;
        while (improved && elapsed() < TIME_LIMIT) {
            improved = false;
            for (int t = prob.T - 1; t >= 0 && elapsed() < TIME_LIMIT; --t) {
                double orig = cur_alpha[t];
                double best_a = orig;
                double local_best = cur_score;

                for (double mult : {exp(delta), exp(-delta)}) {
                    double trial = prob.clamp_alpha(orig * mult);
                    if (trial == orig) continue;
                    cur_alpha[t] = trial;
                    double sc = prob.eval_suffix(cur_alpha, t, cache);
                    if (sc > local_best) {
                        local_best = sc;
                        best_a = trial;
                    }
                }

                cur_alpha[t] = best_a;
                if (best_a != orig) {
                    cur_score = prob.update_cache(cur_alpha, t, cache);
                    if (cur_score > best_score) {
                        best_score = cur_score;
                        best_alpha = cur_alpha;
                    }
                    improved = true;
                }
            }
        }
    }

    // Вывод с 17 значащими цифрами
    cout << setprecision(17);
    for (int t = 0; t < prob.T; ++t) {
        if (t > 0) cout << ' ';
        cout << prob.clamp_alpha(best_alpha[t]);
    }
    cout << '\n';

    return 0;
}
