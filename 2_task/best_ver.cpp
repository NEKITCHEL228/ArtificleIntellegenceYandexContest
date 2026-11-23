#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int W, H;
    if (!(cin >> W >> H)) return 0;

    int N = W * H;
    vector<uint8_t> blocked(N, 0);

    for (int y = 0; y < H; ++y) {
        string row;
        cin >> row;
        int offset = y * W;
        for (int x = 0; x < W; ++x) {
            if (row[x] == '#') {
                blocked[offset + x] = 1;
            }
        }
    }

    vector<int> comp(N, 0);
    vector<int> q(N);
    int cur_comp = 0;

    for (int y = 0; y < H; ++y) {
        int row_offset = y * W;
        for (int x = 0; x < W; ++x) {
            int p = row_offset + x;
            if (blocked[p] || comp[p]) continue;
            ++cur_comp;
            comp[p] = cur_comp;
            int head = 0, tail = 0;
            q[tail++] = p;
            while (head < tail) {
                int u = q[head++];
                int ux = u % W, uy = u / W;
                if (ux > 0) {
                    int v = u - 1;
                    if (!blocked[v] && !comp[v]) { comp[v] = cur_comp; q[tail++] = v; }
                }
                if (ux + 1 < W) {
                    int v = u + 1;
                    if (!blocked[v] && !comp[v]) { comp[v] = cur_comp; q[tail++] = v; }
                }
                if (uy > 0) {
                    int v = u - W;
                    if (!blocked[v] && !comp[v]) { comp[v] = cur_comp; q[tail++] = v; }
                }
                if (uy + 1 < H) {
                    int v = u + W;
                    if (!blocked[v] && !comp[v]) { comp[v] = cur_comp; q[tail++] = v; }
                }
            }
        }
    }

    vector<uint32_t> tag_s(N, 0), tag_t(N, 0);
    vector<int> dist_s(N, 0), dist_t(N, 0);
    vector<int> qs(N), qt(N);
    uint32_t qid = 0;

    int sx, sy, tx, ty;
    while (cin >> sx >> sy >> tx >> ty) {
        if (sx == tx && sy == ty) {
            cout << 0 << '\n' << flush;
            continue;
        }
        int s = sy * W + sx;
        int t = ty * W + tx;

        if (comp[s] == 0 || comp[t] == 0 || comp[s] != comp[t]) {
            cout << -1 << '\n' << flush;
            continue;
        }

        ++qid;
        int hs = 0, ts = 0;
        int ht = 0, tt = 0;

        tag_s[s] = qid; dist_s[s] = 0; qs[ts++] = s;
        tag_t[t] = qid; dist_t[t] = 0; qt[tt++] = t;

        int ans = -1;
        while (hs < ts && ht < tt) {
            if ((ts - hs) <= (tt - ht)) {
                int u = qs[hs++];
                int d = dist_s[u];
                int ux = u % W, uy = u / W;

                if (ux > 0) {
                    int v = u - 1;
                    if (!blocked[v]) {
                        if (tag_t[v] == qid) { ans = d + 1 + dist_t[v]; break; }
                        if (tag_s[v] != qid) { tag_s[v] = qid; dist_s[v] = d + 1; qs[ts++] = v; }
                    }
                }
                if (ux + 1 < W) {
                    int v = u + 1;
                    if (!blocked[v]) {
                        if (tag_t[v] == qid) { ans = d + 1 + dist_t[v]; break; }
                        if (tag_s[v] != qid) { tag_s[v] = qid; dist_s[v] = d + 1; qs[ts++] = v; }
                    }
                }
                if (uy > 0) {
                    int v = u - W;
                    if (!blocked[v]) {
                        if (tag_t[v] == qid) { ans = d + 1 + dist_t[v]; break; }
                        if (tag_s[v] != qid) { tag_s[v] = qid; dist_s[v] = d + 1; qs[ts++] = v; }
                    }
                }
                if (uy + 1 < H) {
                    int v = u + W;
                    if (!blocked[v]) {
                        if (tag_t[v] == qid) { ans = d + 1 + dist_t[v]; break; }
                        if (tag_s[v] != qid) { tag_s[v] = qid; dist_s[v] = d + 1; qs[ts++] = v; }
                    }
                }
                if (ans != -1) break;
            } else {
                int u = qt[ht++];
                int d = dist_t[u];
                int ux = u % W, uy = u / W;

                if (ux > 0) {
                    int v = u - 1;
                    if (!blocked[v]) {
                        if (tag_s[v] == qid) { ans = d + 1 + dist_s[v]; break; }
                        if (tag_t[v] != qid) { tag_t[v] = qid; dist_t[v] = d + 1; qt[tt++] = v; }
                    }
                }
                if (ux + 1 < W) {
                    int v = u + 1;
                    if (!blocked[v]) {
                        if (tag_s[v] == qid) { ans = d + 1 + dist_s[v]; break; }
                        if (tag_t[v] != qid) { tag_t[v] = qid; dist_t[v] = d + 1; qt[tt++] = v; }
                    }
                }
                if (uy > 0) {
                    int v = u - W;
                    if (!blocked[v]) {
                        if (tag_s[v] == qid) { ans = d + 1 + dist_s[v]; break; }
                        if (tag_t[v] != qid) { tag_t[v] = qid; dist_t[v] = d + 1; qt[tt++] = v; }
                    }
                }
                if (uy + 1 < H) {
                    int v = u + W;
                    if (!blocked[v]) {
                        if (tag_s[v] == qid) { ans = d + 1 + dist_s[v]; break; }
                        if (tag_t[v] != qid) { tag_t[v] = qid; dist_t[v] = d + 1; qt[tt++] = v; }
                    }
                }
                if (ans != -1) break;
            }
        }
        cout << ans << '\n' << flush;
    }

    return 0;
}