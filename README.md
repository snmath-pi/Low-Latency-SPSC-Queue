# Low-Latency Lock-Free SPSC Ring Buffer

A sub-microsecond, hardware-optimized Single-Producer, Single-Consumer (SPSC) lock-free queue written in C++17. Designed for zero-allocation thread communication in High-Frequency Trading (HFT) systems.

## Architecture & Systems Engineering
This queue completely bypasses the Operating System (no `std::mutex`, no locks, no system calls) and relies entirely on bare-metal hardware synchronization.

* **Zero Pointer Indirection:** Uses a flat, pre-allocated `std::array` instead of heap-allocated vectors to guarantee perfect L1 cache spatial locality and zero page faults during execution.
* **Cache Line Alignment:** The `head`, `tail`, and `buffer` are explicitly aligned to 64-byte boundaries using `alignas(64)` to strictly prevent CPU **False Sharing** between the producer and consumer cores.
* **Acquire/Release Semantics:** Uses C++11 lightweight `std::memory_order_acquire` and `std::memory_order_release` to create a strict memory barrier handshake, avoiding the heavy latency penalties of sequentially consistent (`seq_cst`) atomics.
* **Bitwise Wrap-Around:** Enforces a Power-of-2 capacity at compile-time (`static_assert`), replacing the standard 20-cycle Modulo (`%`) integer division with a 1-clock-cycle bitwise AND (`&`) mask.

## Benchmark Performance
*Tested on 10,000,000 continuous integers passing between two isolated CPU cores.*

| Metric | Result |
| :--- | :--- |
| **Throughput** | 20M - 30M+ messages / second |
| **Average Latency** | < 50 nanoseconds / message |
| **Memory Allocations** | 0 (after initialization) |

## 🛠️ Build & Run

To achieve these nanosecond latencies, you must compile with maximum optimizations (`-O3`) and enable multithreading.

```bash
# Compile the benchmark
g++ -O3 -pthread -std=c++17 main.cpp -o benchmark

# Execute
./benchmark