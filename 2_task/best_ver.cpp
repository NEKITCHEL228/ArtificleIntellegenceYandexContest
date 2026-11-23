#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <array>

using namespace std;

// Fast I/O primitives
#if defined(__APPLE__)
#ifndef getchar_unlocked
#define getchar_unlocked getchar
#endif
#ifndef putchar_unlocked
#define putchar_unlocked putchar
#endif
#endif

static inline bool fast_read_int(int &out) {
    int c = getchar_unlocked();
    while (c <= ' ' && c != EOF) {
        c = getchar_unlocked();
    }
    if (c == EOF) return false;
    out = c - '0';
    while ((c = getchar_unlocked()) >= '0' && c <= '9') {
        out = out * 10 + (c - '0');
    }
    return true;
}

static inline void fast_write_int(int x) {
    char buf[16];
    char *p = buf + 15;
    *p = '\n';
    if (x == 0) {
        *--p = '0';
    } else if (x < 0) {
        *--p = '1';
        *--p = '-';
    } else {
        while (x > 0) {
            *--p = (char)('0' + (x % 10));
            x /= 10;
        }
    }
    fwrite(p, 1, (buf + 16) - p, stdout);
    fflush(stdout);
}

int main() {
    int W, H;
    if (!fast_read_int(W) || !fast_read_int(H)) return 0;

    const int stride = W + 2;
    const int H_pad = H + 2;
    const int N_pad = stride * H_pad;

    // 1-cell border padding around the grid to eliminate all boundary checks
    vector<uint8_t> blocked(N_pad, 1);
    for (int y = 0; y < H; ++y) {
        int row_offset = (y + 1) * stride + 1;
        for (int x = 0; x < W; ++x) {
            int c = getchar_unlocked();
            while (c != '.' && c != '#') c = getchar_unlocked();
            if (c == '.') {
                blocked[row_offset + x] = 0;
            }
        }
    }

    // Connected components labeling
    vector<int> comp(N_pad, 0);
    vector<int> q(N_pad);
    vector<int> comp_size(N_pad + 1, 0);
    int cur_comp = 0;
    const int d_offset[4] = {-1, 1, -stride, stride};
    const int dx[4] = {-1, 1, 0, 0};
    const int dy[4] = {0, 0, -1, 1};

    for (int y = 0; y < H; ++y) {
        int row_offset = (y + 1) * stride + 1;
        for (int x = 0; x < W; ++x) {
            int p = row_offset + x;
            if (blocked[p] || comp[p]) continue;
            ++cur_comp;
            comp[p] = cur_comp;
            int head = 0, tail = 0;
            q[tail++] = p;
            while (head < tail) {
                int u = q[head++];
                for (int d = 0; d < 4; ++d) {
                    int v = u + d_offset[d];
                    if (!blocked[v] && !comp[v]) {
                        comp[v] = cur_comp;
                        q[tail++] = v;
                    }
                }
            }
            comp_size[cur_comp] = tail;
        }
    }

    // Identify main connected component
    int main_comp = 1;
    for (int c = 2; c <= cur_comp; ++c) {
        if (comp_size[c] > comp_size[main_comp]) {
            main_comp = c;
        }
    }

    // --- FARTHEST POINT SAMPLING (FPS) LANDMARK SELECTION ---
    const int K = 16;
    vector<int> landmarks;
    landmarks.reserve(K);

    // Initial landmark: top-left reachable cell of main component
    int first_lm = -1;
    for (int p = 0; p < N_pad; ++p) {
        if (comp[p] == main_comp) {
            first_lm = p;
            break;
        }
    }

    if (first_lm != -1) {
        landmarks.push_back(first_lm);
    }

    // Precomputed distances from each landmark (flat array: K * N_pad ints)
    vector<int> lm_flat(K * N_pad, -1);
    vector<int> min_d(N_pad, 1e9);

    for (int k = 0; k < K && !landmarks.empty(); ++k) {
        int lm = landmarks[k];
        int* cur_dist = &lm_flat[k * N_pad];

        // BFS from lm
        int head = 0, tail = 0;
        q[tail++] = lm;
        cur_dist[lm] = 0;

        while (head < tail) {
            int u = q[head++];
            int d = cur_dist[u] + 1;
            for (int dir = 0; dir < 4; ++dir) {
                int v = u + d_offset[dir];
                if (!blocked[v] && cur_dist[v] == -1) {
                    cur_dist[v] = d;
                    q[tail++] = v;
                }
            }
        }

        // Update min_d and find next farthest landmark via MaxMin criterion
        if (k + 1 < K) {
            int next_lm = -1;
            int max_dist = -1;
            for (int p = 0; p < N_pad; ++p) {
                if (cur_dist[p] != -1) {
                    if (cur_dist[p] < min_d[p]) min_d[p] = cur_dist[p];
                    if (min_d[p] > max_dist) {
                        max_dist = min_d[p];
                        next_lm = p;
                    }
                }
            }
            if (next_lm != -1 && max_dist > 0) {
                landmarks.push_back(next_lm);
            } else {
                break;
            }
        }
    }

    const int num_landmarks = (int)landmarks.size();

    // 4 circular buckets for Dial's A*
    vector<int> bucket[4];
    for (int i = 0; i < 4; ++i) bucket[i].reserve(16384);

    vector<uint32_t> tag(N_pad, 0);
    vector<int> dist(N_pad, 0);
    uint32_t qid = 0;

    int sx, sy, tx, ty;
    while (fast_read_int(sx)) {
        if (!fast_read_int(sy) || !fast_read_int(tx) || !fast_read_int(ty)) break;

        const int diff_x = abs(sx - tx);
        const int diff_y = abs(sy - ty);
        const int manhattan = diff_x + diff_y;

        if (manhattan == 0) {
            fast_write_int(0);
            continue;
        }

        const int s = (sy + 1) * stride + (sx + 1);
        const int t = (ty + 1) * stride + (tx + 1);

        if (comp[s] == 0 || comp[t] == 0 || comp[s] != comp[t]) {
            fast_write_int(-1);
            continue;
        }

        if (manhattan == 1) {
            fast_write_int(1);
            continue;
        }

        if (manhattan == 2) {
            if (diff_x == 1 && diff_y == 1) {
                int mid1 = (sy + 1) * stride + (tx + 1);
                int mid2 = (ty + 1) * stride + (sx + 1);
                if (!blocked[mid1] || !blocked[mid2]) {
                    fast_write_int(2);
                    continue;
                }
            } else if (diff_x == 2) {
                int mid = (sy + 1) * stride + ((sx + tx) / 2 + 1);
                if (!blocked[mid]) {
                    fast_write_int(2);
                    continue;
                }
            } else if (diff_y == 2) {
                int mid = (((sy + ty) / 2) + 1) * stride + (sx + 1);
                if (!blocked[mid]) {
                    fast_write_int(2);
                    continue;
                }
            }
        }

        // --- LANDMARK LOWER BOUND & TRIANGLE INEQUALITY SHORTCUT ---
        int best_lb = manhattan;
        int best_ub = 1e9;
        int active_lm1 = -1, active_lm2 = -1;
        int max_lb1 = -1, max_lb2 = -1;

        for (int i = 0; i < num_landmarks; ++i) {
            const int* d_ptr = &lm_flat[i * N_pad];
            int d_s = d_ptr[s];
            int d_t = d_ptr[t];
            if (d_s != -1 && d_t != -1) {
                int diff_lb = abs(d_s - d_t);
                int sum_ub = d_s + d_t;
                if (diff_lb > best_lb) best_lb = diff_lb;
                if (sum_ub < best_ub) best_ub = sum_ub;

                if (diff_lb > max_lb1) {
                    max_lb2 = max_lb1;
                    active_lm2 = active_lm1;
                    max_lb1 = diff_lb;
                    active_lm1 = i;
                } else if (diff_lb > max_lb2) {
                    max_lb2 = diff_lb;
                    active_lm2 = i;
                }
            }
        }

        // Instant O(1) shortcut if a landmark lies on the shortest path
        if (best_lb == best_ub) {
            fast_write_int(best_lb);
            continue;
        }

        ++qid;
        int ans = -1;

        // Path A: If landmarks provide no tighter bound than Manhattan,
        // use branch-free pure Manhattan A* to avoid landmark cache lines
        if (best_lb == manhattan) {
            int cur_f = manhattan;
            dist[s] = 0;
            tag[s] = qid;
            bucket[cur_f & 3].push_back(s);
            int open_count = 1;

            while (open_count > 0) {
                while (bucket[cur_f & 3].empty()) {
                    cur_f++;
                }

                int u = bucket[cur_f & 3].back();
                bucket[cur_f & 3].pop_back();
                open_count--;

                int d = dist[u];
                int ux = (u % stride) - 1;
                int uy = (u / stride) - 1;

                if (u == t) {
                    ans = d;
                    break;
                }

                int h_u = abs(ux - tx) + abs(uy - ty);
                if (d + h_u < cur_f) continue;

                const int d_next = d + 1;
                #pragma unroll
                for (int i = 0; i < 4; ++i) {
                    int v = u + d_offset[i];
                    if (!blocked[v]) {
                        int vx = ux + dx[i];
                        int vy = uy + dy[i];
                        int h_v = abs(vx - tx) + abs(vy - ty);
                        int f_v = d_next + h_v;

                        if (tag[v] != qid || d_next < dist[v]) {
                            tag[v] = qid;
                            dist[v] = d_next;
                            bucket[f_v & 3].push_back(v);
                            open_count++;
                        }
                    }
                }
            }
            for (int i = 0; i < 4; ++i) bucket[i].clear();
        } else {
            // Path B: Landmark-guided search with top 2 active landmarks
            const int lm1_t = (active_lm1 != -1) ? lm_flat[active_lm1 * N_pad + t] : -1;
            const int lm2_t = (active_lm2 != -1) ? lm_flat[active_lm2 * N_pad + t] : -1;
            const int* dist_lm1 = (active_lm1 != -1) ? &lm_flat[active_lm1 * N_pad] : nullptr;
            const int* dist_lm2 = (active_lm2 != -1) ? &lm_flat[active_lm2 * N_pad] : nullptr;

            auto calc_h = [&](int u, int ux, int uy) -> int {
                int h = abs(ux - tx) + abs(uy - ty);
                if (dist_lm1 && dist_lm1[u] != -1) {
                    int h1 = abs(dist_lm1[u] - lm1_t);
                    if (h1 > h) h = h1;
                }
                if (dist_lm2 && dist_lm2[u] != -1) {
                    int h2 = abs(dist_lm2[u] - lm2_t);
                    if (h2 > h) h = h2;
                }
                return h;
            };

            int cur_f = calc_h(s, sx, sy);
            dist[s] = 0;
            tag[s] = qid;
            bucket[cur_f & 3].push_back(s);
            int open_count = 1;

            while (open_count > 0) {
                while (bucket[cur_f & 3].empty()) {
                    cur_f++;
                }

                int u = bucket[cur_f & 3].back();
                bucket[cur_f & 3].pop_back();
                open_count--;

                int d = dist[u];
                int ux = (u % stride) - 1;
                int uy = (u / stride) - 1;

                if (u == t) {
                    ans = d;
                    break;
                }

                int h_u = calc_h(u, ux, uy);
                if (d + h_u < cur_f) continue;

                const int d_next = d + 1;
                #pragma unroll
                for (int i = 0; i < 4; ++i) {
                    int v = u + d_offset[i];
                    if (!blocked[v]) {
                        int vx = ux + dx[i];
                        int vy = uy + dy[i];
                        int h_v = calc_h(v, vx, vy);
                        int f_v = d_next + h_v;

                        if (tag[v] != qid || d_next < dist[v]) {
                            tag[v] = qid;
                            dist[v] = d_next;
                            bucket[f_v & 3].push_back(v);
                            open_count++;
                        }
                    }
                }
            }
            for (int i = 0; i < 4; ++i) bucket[i].clear();
        }

        fast_write_int(ans);
    }

    return 0;
}
