---
paths:
  - "**/*.cpp"
  - "**/*.hpp"
  - "**/*.cc"
  - "**/*.hh"
  - "**/*.cxx"
  - "**/*.h"
---
# C++ Coding Standards & Competitive Best Practices

> Extends [common/coding-style.md](../common/coding-style.md) and tailored for high-performance competitive programming and AI algorithmic kernels.

---

## 1. Modern C++ Standard: C++20 (GCC 14.1 Target)

- Target compiler & standard: **GCC 14.1 with `-std=c++20`** (competition testing environment).
- Optimization flags:
  ```bash
  g++ -O3 -std=c++20 -march=native -Wall -Wextra solution.cpp -o solution
  ```
- **Leverage C++20 Features**:
  - **`std::span` (`<span>`)**: Use for zero-overhead non-owning views over contiguous memory (`vector`, array) instead of deep copies or passing raw pointer + length pairs.
  - **Standard bit manipulation (`<bit>`)**: Use `std::popcount`, `std::countl_zero`, `std::bit_width` instead of compiler-specific `__builtin_*` functions.
  - **Designated Initializers**: For clear, self-documenting POD structs:
    ```cpp
    struct Point { int x; int y; int cost; };
    Point p{.x = 10, .y = 20, .cost = 0};
    ```
  - **Structured Bindings & Spaceship Operator**:
    ```cpp
    auto [x, y, cost] = queue.top();
    auto operator<=>(const Point&) const = default;
    ```
  - **`std::numbers` (`<numbers>`)**: Use `std::numbers::pi` for exact mathematical constants.
  - **Ranges & Views (`<ranges>`)**: Use where readability is aided without overhead in hot paths.

---

## 2. Competitive Fast I/O Discipline

In any competition program reading or writing large amounts of data:
```cpp
std::ios_base::sync_with_stdio(false);
std::cin.tie(nullptr);
```
- Use `'\n'` instead of `std::endl` to prevent unintended buffer flushing.
- If handling millions of numbers, prefer reading into pre-allocated memory.

---

## 3. Memory & Cache Locality (Anti-TLE / Anti-MLE)

1. **Contiguous Memory over Pointer Chasing**:
   - Prefer flat 1D `std::vector` or arrays for 2D/3D grids (`grid[y * W + x]`) instead of `std::vector<std::vector<T>>` to maximize CPU L1/L2 cache hits.
   - Use compact data types for masks and visited tables:
     - `uint8_t` or `std::vector<uint8_t>` / `std::bitset<N>` instead of 4-byte `int` or `std::vector<bool>` (which has proxy reference overhead).
2. **Zero Dynamic Allocation in Hot Loops**:
   - Pre-allocate with `.reserve()` or initialize containers once outside the query/iteration loops.
   - Reuse buffers (e.g. static BFS queues or reusable scratch vectors) rather than allocating inside loops.
3. **Pass by Reference**:
   - Always pass containers, strings, and non-trivial structs by `const &`:
     ```cpp
     void process_scene(const std::vector<Point>& points); // GOOD
     void process_scene(std::vector<Point> points);        // BAD - deep copies data
     ```
   - In range-based loops, always use `const auto&` unless a copy or mutation is explicitly required:
     ```cpp
     for (const auto& item : items) { ... }
     ```

---

## 4. Integer Overflow & Numerical Precision

- For sums, products, coordinates, squared distances, and combinatorics, ALWAYS use `int64_t` or `long long` to prevent silent overflow.
- Float comparisons: never compare floating point numbers with `==`. Use epsilon tolerances:
  ```cpp
  constexpr double EPS = 1e-9;
  if (std::abs(a - b) < EPS) { ... }
  ```

---

## 5. Safety & Undefined Behavior (UB) Prevention

- Initialize all variables and stack arrays upon declaration.
- Assert bounds before accessing raw arrays: `assert(idx >= 0 && idx < N);`.
- Avoid undefined shifts: `1LL << k` instead of `1 << k` when `k >= 31`.
- Clean error handling: exit gracefully on malformed or empty input lines.
