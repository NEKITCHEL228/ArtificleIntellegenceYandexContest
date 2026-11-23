#!/usr/bin/env python3
import subprocess
import time
import random
import sys
from pathlib import Path

WORKSPACE = Path(__file__).resolve().parent

def generate_open_grid(W, H, obstacle_prob=0.10, seed=42):
    random.seed(seed)
    grid = []
    free = []
    for y in range(H):
        row = []
        for x in range(W):
            if random.random() < obstacle_prob and not (x == 0 and y == 0):
                row.append('#')
            else:
                row.append('.')
                free.append((x, y))
        grid.append("".join(row))
    return grid, free

def generate_rooms_grid(W, H, num_rooms=25, seed=42):
    random.seed(seed)
    grid = [['#' for _ in range(W)] for _ in range(H)]
    rooms = []
    for _ in range(num_rooms):
        rw = random.randint(15, min(W // 4, 60))
        rh = random.randint(15, min(H // 4, 60))
        rx = random.randint(1, W - rw - 1)
        ry = random.randint(1, H - rh - 1)
        rooms.append((rx, ry, rw, rh))
        for y in range(ry, ry + rh):
            for x in range(rx, rx + rw):
                grid[y][x] = '.'
    # Connect adjacent rooms with corridors
    for i in range(len(rooms) - 1):
        x1, y1 = rooms[i][0] + rooms[i][2] // 2, rooms[i][1] + rooms[i][3] // 2
        x2, y2 = rooms[i+1][0] + rooms[i+1][2] // 2, rooms[i+1][1] + rooms[i+1][3] // 2
        for x in range(min(x1, x2), max(x1, x2) + 1):
            grid[y1][x] = '.'
        for y in range(min(y1, y2), max(y1, y2) + 1):
            grid[y][x2] = '.'
    free = [(x, y) for y in range(H) for x in range(W) if grid[y][x] == '.']
    return ["".join(row) for row in grid], free

def generate_maze_grid(W, H, seed=42):
    random.seed(seed)
    # Ensure odd dimensions for classic maze
    W_adj = W if W % 2 == 1 else W - 1
    H_adj = H if H % 2 == 1 else H - 1
    grid = [['#' for _ in range(W)] for _ in range(H)]
    
    # DFS maze
    stack = [(1, 1)]
    grid[1][1] = '.'
    visited = set([(1, 1)])
    while stack:
        cx, cy = stack[-1]
        neighbors = []
        for dx, dy in [(-2, 0), (2, 0), (0, -2), (0, 2)]:
            nx, ny = cx + dx, cy + dy
            if 1 <= nx < W_adj and 1 <= ny < H_adj and (nx, ny) not in visited:
                neighbors.append((nx, ny, cx + dx // 2, cy + dy // 2))
        if neighbors:
            nx, ny, wx, wy = random.choice(neighbors)
            grid[wy][wx] = '.'
            grid[ny][nx] = '.'
            visited.add((nx, ny))
            stack.append((nx, ny))
        else:
            stack.pop()
            
    # Add some random shortcuts so it's not a pure tree (braid maze)
    for _ in range(W * H // 100):
        rx = random.randint(1, W - 2)
        ry = random.randint(1, H - 2)
        grid[ry][rx] = '.'
        
    free = [(x, y) for y in range(H) for x in range(W) if grid[y][x] == '.']
    return ["".join(row) for row in grid], free

def benchmark_map(name, grid, free, num_queries=2000, seed=123):
    H = len(grid)
    W = len(grid[0])
    random.seed(seed)
    queries = []
    for _ in range(num_queries):
        s = random.choice(free)
        t = random.choice(free)
        queries.append(f"{s[0]} {s[1]} {t[0]} {t[1]}")
    input_data = f"{W} {H}\n" + "\n".join(grid) + "\n" + "\n".join(queries) + "\n"

    print(f"\n================ Map: {name} ({W}x{H}, {num_queries} queries) ================")
    binaries = [
        ("Baseline (best_ver)", WORKSPACE / "best_ver_orig"),
        ("ALT (test_alt)", WORKSPACE / "test_alt"),
        ("ALT Hybrid (test_alt_hybrid)", WORKSPACE / "test_alt_hybrid"),
        ("ALT FPS (test_fps)", WORKSPACE / "test_fps"),
    ]
    results = {}
    for label, bpath in binaries:
        if not bpath.exists():
            continue
        t0 = time.perf_counter()
        p = subprocess.run([str(bpath)], input=input_data, text=True, capture_output=True)
        dur = time.perf_counter() - t0
        out = p.stdout.strip().split()
        if len(out) != num_queries:
            print(f"  [{label}] ERROR: expected {num_queries} outputs, got {len(out)}")
            continue
        qps = num_queries / dur
        results[label] = (dur, qps, out)
        print(f"  {label:<30}: {dur:.3f} s  ({qps:8.1f} qps)")

    # Verify matching
    base_out = results.get("Baseline (best_ver)", (None, None, None))[2]
    if base_out:
        for label, (_, _, out) in results.items():
            if label == "Baseline (best_ver)":
                continue
            mismatches = sum(1 for a, b in zip(base_out, out) if a != b)
            if mismatches > 0:
                print(f"  WARNING: {label} has {mismatches} mismatches vs Baseline!")
            else:
                print(f"  Verification: {label} 100% matches Baseline.")

def main():
    # Compile
    print("Compiling test binaries...")
    subprocess.run(["g++", "-O3", "-std=c++20", str(WORKSPACE / "best_ver.cpp"), "-o", str(WORKSPACE / "best_ver_orig")], check=True)
    subprocess.run(["g++", "-O3", "-std=c++20", str(WORKSPACE / "test_alt.cpp"), "-o", str(WORKSPACE / "test_alt")], check=True)
    subprocess.run(["g++", "-O3", "-std=c++20", str(WORKSPACE / "test_alt_hybrid.cpp"), "-o", str(WORKSPACE / "test_alt_hybrid")], check=True)

    # 1. Open grid
    g_open, f_open = generate_open_grid(800, 800, obstacle_prob=0.10)
    benchmark_map("Open Grid 800x800 (10% obstacles)", g_open, f_open, num_queries=3000)

    # 2. Obstacle dense grid (25% obstacles)
    g_dense, f_dense = generate_open_grid(800, 800, obstacle_prob=0.25)
    benchmark_map("Dense Obstacles 800x800 (25% obstacles)", g_dense, f_dense, num_queries=3000)

    # 3. Rooms & Corridors
    g_rooms, f_rooms = generate_rooms_grid(800, 800, num_rooms=35)
    benchmark_map("Rooms & Corridors 800x800", g_rooms, f_rooms, num_queries=3000)

    # 4. Maze (801x801)
    g_maze, f_maze = generate_maze_grid(801, 801)
    benchmark_map("Maze 801x801 (long detours)", g_maze, f_maze, num_queries=3000)

if __name__ == "__main__":
    main()
