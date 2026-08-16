# Multi-Threaded Sum of Three Cubes Solver

A C++ implementation for finding integer solutions to the Diophantine equation:

$$
x^3 + y^3 + z^3 = n
$$

This repository provides a multi-threaded brute-force search algorithm utilizing lock-free atomic synchronization for efficient parallel execution across available CPU cores.

## Project Structure

```text
.
├── CMakeLists.txt
├── README.md
├── LICENSE
├── .gitignore
└── src/
    └── main.cpp
```

## Overview

The problem of expressing an integer $n$ as the sum of three cubes,

$$
x^3 + y^3 + z^3 = n
$$

is a classic Diophantine equation. Solutions do not exist for integers satisfying:

$$
n \equiv 4 \pmod 9
$$

or

$$
n \equiv 5 \pmod 9
$$

For other integers, solutions can be extremely sparse and may require an enormous search space to locate.

This program uses a brute-force search strategy and partitions the search space across multiple threads. The number of worker threads is determined from the available hardware concurrency.

The goal of this project is not to compete with specialized algorithms such as advanced sieves, but to explore how far a straightforward brute-force approach can be pushed through parallelism, atomic synchronization, mathematical pruning, and early termination.

## Features

- **Multi-Threaded Brute-Force Search:** The search space is divided among multiple worker threads.
- **Lock-Free Synchronization:** Uses `std::atomic<int>` without mutex-based locking during the search.
- **Low-Overhead State Checking:** Worker threads use `std::memory_order_relaxed` when polling the shared search state.
- **Atomic Solution Claiming:** Uses `compare_exchange_strong` to ensure that only one thread can claim a discovered solution.
- **Early Exit:** Once a solution is found, all worker threads terminate their search as soon as they observe the shared state change.
- **Mathematical Pruning:** Values that cannot have a solution are rejected immediately using the modulo 9 constraint.

## Synchronization Mechanics

The solver uses a shared atomic state variable to coordinate the worker threads.

The state represents the current status of the search:

| State | Meaning |
|---|---|
| `1` | Search is in progress |
| `-1` | A solution of the form $x^3 + y^3 + z^3$ was found |
| `-2` | A solution of the form $-x^3 + y^3 + z^3$ was found |
| `-3` | A solution of the form $x^3 - y^3 - z^3$ was found |

### Early Exit

Worker threads continually inspect the shared atomic state:

```cpp
for (u64 y = a;
     y <= b && state.load(std::memory_order_relaxed) == 1;
     ++y) {
    // Search loop
}
```

When any thread finds a valid solution, it changes the shared state. The remaining threads then leave their search loops as soon as they observe the change.

This prevents the program from continuing an expensive brute-force search after a solution has already been found.

### Atomic Solution Claim

When a worker discovers a valid solution, it attempts to claim the result using an atomic Compare-And-Swap operation:

```cpp
int expected = 1;

if (state.compare_exchange_strong(
        expected,
        -3,
        std::memory_order_acq_rel)) {

    fk = t;
    fl = y;
    fm = z;
}
```

Only the first thread that successfully changes the state from `1` to the corresponding solution state is allowed to write the result.

This prevents multiple worker threads from simultaneously claiming different solutions.

## Search Strategy

The solver intentionally uses a brute-force approach rather than a sieve or another specialized number-theoretic algorithm.

The search proceeds in several stages:

1. Reject values that cannot have a solution using the modulo 9 constraint.
2. Search for solutions involving three positive cubes.
3. Expand the search range to signed combinations.
4. Partition the search space across multiple worker threads.
5. Terminate all workers as soon as one thread discovers a valid solution.

The implementation focuses on making brute force as efficient as possible while preserving the simplicity of the underlying search strategy.

## Building and Running

### Prerequisites

- C++11 or later
- GCC, Clang, or MSVC
- CMake 3.10 or later

### Build Instructions

Generate the build configuration:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

Build the project:

```bash
cmake --build build --config Release
```

The compiled binary will be placed inside the `build` directory.

## Example Output

The following example shows a run for $n = 39$ on an Intel Core i7-3770 (4 cores / 8 threads):

```text
Input number: 39
117367^3 - 159380^3 + 134476^3 = 39
Time elapsed: 1049.019 seconds
```

The corresponding identity is:

$$
39 = 117367^3 - 159380^3 + 134476^3
$$

The solution for $39$ was first discovered in September 2019 by Andrew Booker using the BlueCrystal Phase 3 supercomputer.

## Performance

This implementation is **not intended to compete with state-of-the-art algorithms** for the sum of three cubes problem.

Its purpose is to demonstrate how a computationally expensive brute-force search can be improved using:

- CPU parallelism
- Atomic synchronization
- Early termination
- Mathematical constraints
- Search-space partitioning

Performance will vary depending on CPU architecture, number of threads, compiler, optimization level, and the particular value of $n$.

## License

See [LICENSE](LICENSE) for details.
