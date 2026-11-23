---
paths:
  - "**/*.cpp"
  - "**/*.hpp"
  - "**/*.cc"
  - "**/*.hh"
  - "**/*.cxx"
  - "**/*.h"
---
# C++ Algorithmic & Performance Patterns

> Extends [coding-style.md](coding-style.md) for competitive programming and ML kernel optimization.

---

## 1. Grid & Spatial Flattening Pattern

Always map 2D coordinates `(x, y)` in a grid of width `W` to a 1D index:
```cpp
inline int get_index(int x, int y, int W) {
    return y * W + x;
}
```
*Rationale*: A 1D vector `std::vector<T> data(W * H)` keeps the entire grid contiguous in RAM, reducing cache misses by up to 5–10x compared to `vector<vector<T>>`.

---

## 2. Reusable BFS/DFS Queue Pattern

Avoid constructing and destroying queues in recurring queries:
```cpp
// Allocate queue and visited buffer once
std::vector<int> q(N);
std::vector<int> visited(N, 0);
int iteration_token = 1;

void run_bfs(int start_node, int N) {
    int head = 0, tail = 0;
    q[tail++] = start_node;
    visited[start_node] = iteration_token;

    while (head < tail) {
        int u = q[head++];
        // Process u...
        // For neighbors:
        // if (visited[v] != iteration_token) {
        //     visited[v] = iteration_token;
        //     q[tail++] = v;
        // }
    }
    iteration_token++; // Reset visited without memset(0)
}
```

---

## 3. Fast Bitmask & Coordinate Operations (C++20 `<bit>`)

In C++20 (GCC 14.1), always use the standard, hardware-accelerated `<bit>` header instead of legacy macros:
```cpp
#include <bit>

int count = std::popcount(mask);        // Hardware POPCNT
int leading_zeros = std::countl_zero(mask);
int trailing_zeros = std::countr_zero(mask);
bool is_pow2 = std::has_single_bit(mask);
int bit_width = std::bit_width(mask);
```

---

## 4. Compilation & Sanitization Command (GCC 14.1 / C++20)

- **Target Competition Build (GCC 14.1, C++20)**:
  ```bash
  g++ -O3 -std=c++20 -march=native -Wall -Wextra -Wshadow solution.cpp -o solution
  ```

- **Local Debug & UB-Sanitization Build**:
  ```bash
  g++ -O2 -std=c++20 -fsanitize=address,undefined -Wall -Wextra solution.cpp -o solution_debug
  ```
  *(На macOS локальный компилятор `clang++ -std=c++20` также поддерживает стандарт C++20).*
