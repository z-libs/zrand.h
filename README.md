
# zrand.h

`zrand.h` is a high-performance, cross-platform Random Number Generator (RNG) library for C and C++. It implements the **PCG-XSH-RR** algorithm, which offers statistically superior randomness compared to the standard `rand()` (Linear Congruential Generator) while remaining extremely fast and memory-efficient.

It also includes a robust **C++11 wrapper** (`z_rand` namespace) that integrates with STL containers and provides a modern, fluent API.

## Features

* **Statistically Robust**: Uses the PCG algorithm (Permuted Congruential Generator). Passes rigorous statistical tests (TestU01/BigCrush) where standard `rand()` fails.
* **Bias-Free Ranges**: Implements rejection sampling for `zrand_range()`, eliminating the "modulo bias" inherent in `rand() % N`.
* **Cross-Platform Entropy**: Automatically seeds from the OS CSPRNG (`/dev/urandom` on Linux/macOS, `rand_s` on Windows).
* **Thread Safety**: Uses thread-local state by default. No locks required for global generation.
* **Deterministic Mode**: Supports explicit seed/sequence initialization for replay systems or procedural generation.
* **Zero Dependencies**: Only standard C headers used.
* **Header Only**: No linking required.

## Installation

1.  Copy `zrand.h` to your project's include folder.
2.  In exactly **one** source file (`.c` or `.cpp`), define the implementation macro `ZRAND_IMPLEMENTATION` before including the header.

```c
#define ZRAND_IMPLEMENTATION
#include "zrand.h"
```

## Usage: C

For C projects, you can use the global API for immediate results without managing state. The library handles seeding automatically.

```c
#include <stdio.h>
#include "zrand.h"

int main(void)
{
    // Auto-seeds from OS entropy on first use. No srand() needed.
    
    // Basic generation.
    printf("d20 Roll: %d\n", zrand_range(1, 20));
    printf("Float (0..1): %.4f\n", zrand_f32());

    // Utilities.
    if (zrand_chance(0.05)) 
    {
        printf("Critical Hit! (5%% chance)\n");
    }

    // UUID generation.
    char uuid[37];
    zrand_uuid(uuid);
    printf("New ID: %s\n", uuid);

    return 0;
}
```

## Usage: C++

The library detects C++ automatically. The wrapper lives in the **`z_rand`** namespace.

```cpp
#include <iostream>
#include <vector>
#include "zrand.h"

int main()
{
    // A native-like API.
    std::cout << "d6 Roll: " << z_rand::range(1, 6) << "\n";
    std::cout << "UUID: " << z_rand::uuid() << "\n"; // Returns std::string.

    // STL integration.
    std::vector<int> deck = {1, 2, 3, 4, 5};
    z_rand::shuffle(deck); // Works directly on std::vector.

    // Deterministic Instance.
    z_rand::generator rng(12345); // Fixed seed.
    std::cout << "Replay Value: " << rng.range(1, 100) << "\n";

    return 0;
}
```

## API Reference (C)

### Global Generation
These functions use a high-performance **thread-local** generator instance. It is automatically seeded from the OS on the first call in each thread.

**Core Generators**

| Function | Description |
| :--- | :--- |
| `zrand_u32()` | Returns a random 32-bit unsigned integer. |
| `zrand_u64()` | Returns a random 64-bit unsigned integer. |
| `zrand_f32()` | Returns a `float` in the range `[0.0, 1.0)` (exclusive). |
| `zrand_f64()` | Returns a `double` in the range `[0.0, 1.0)` (exclusive). |
| `zrand_bool()` | Returns `true` or `false` (50/50 probability). |

**Utilities**

| Function | Description |
| :--- | :--- |
| `zrand_range(min, max)` | Returns `int32_t` in range `[min, max]` (inclusive). Bias-free. |
| `zrand_range_f(min, max)` | Returns `float` in range `[min, max)`. |
| `zrand_chance(probability)`| Returns `true` if a random check passes the given probability (0.0 to 1.0). |
| `zrand_bytes(buf, len)` | Fills a buffer with random bytes. |
| `zrand_str(buf, len)` | Fills a buffer with a random alphanumeric string (A-Z, a-z, 0-9). |
| `zrand_uuid(buf)` | Generates a valid **UUID v4** string. `buf` must be at least 37 bytes. |
| `zrand_shuffle(ptr, n, sz)` | Shuffles an array in-place using Fisher-Yates. |

**Management**

| Function | Description |
| :--- | :--- |
| `zrand_init()` | Explicitly re-seeds the current thread's generator from OS entropy (`/dev/urandom` or `rand_s`). |

### Instance API (Deterministic)
Use these functions when you need reproducible sequences (e.g., for game replays, procedural generation, or simulations) where the randomness must be identical given the same seed.

**Management**

| Function | Description |
| :--- | :--- |
| `zrand_rng_init(rng, seed, seq)` | Initializes a specific `zrand_rng` struct. `seq` (sequence) allows different streams from the same seed. |

**Generation**

| Function | Description |
| :--- | :--- |
| `zrand_rng_u32(rng)` | Helper to generate a 32-bit int from a specific instance. |
| `zrand_rng_u64(rng)` | Helper to generate a 64-bit int from a specific instance. |
| `zrand_rng_f64(rng)` | Helper to generate a double from a specific instance. |
| `zrand_rng_range(rng, min, max)` | Helper to generate a range from a specific instance. |

## API Reference (C++)

The C++ wrapper is defined in the **`z_rand`** namespace.

**Global Shortcuts**

| Function | Description |
| :--- | :--- |
| `z_rand::init()` | Wraps `zrand_init`. |
| `z_rand::u32()`, `u64()` | Returns random integers. |
| `z_rand::f32()`, `f64()` | Returns random floating point numbers. |
| `z_rand::boolean()` | Returns boolean. |
| `z_rand::chance(prob)` | Returns true based on probability. |
| `z_rand::range(min, max)` | Returns value in range (overloaded for `int` and `float`). |
| `z_rand::uuid()` | Returns a standard `std::string` containing a UUID v4. |
| `z_rand::string(len)` | Returns a standard `std::string` of random alphanumeric characters. |
| `z_rand::shuffle(vector)` | Shuffles a `std::vector` (or `z_vec::vector`) in-place. |

**Deterministic Generator**

The `z_rand::generator` class wraps the C struct state (`zrand_rng`). It provides methods matching the global API but operates on its own internal state.

```cpp
// Seed 1234, Sequence 1.
z_rand::generator rng(1234);

int hp = rng.range(50, 100);
bool has_key = rng.chance(0.25);
```

## Why PCG?

The default `rand()` in C is typically a Linear Congruential Generator (LCG). LCGs have statistical flaws:
1.  **Short Periods**: They repeat sequences relatively quickly.
2.  **Poor Distribution**: They can fail basic statistical tests (like the "Birthday Spacings" test).
3.  **Low Bits**: The lower bits of an LCG often exhibit visible patterns (odd/even/odd/even).

**PCG (Permuted Congruential Generator)** solves this by applying a permutation function (XSH-RR) to the output of an LCG. This scrambles the bits, resulting in:
* Excellent statistical quality (passes BigCrush).
* Performance that is often **faster** than `rand()` due to modern CPU optimizations.
* A huge period (2^64), making repeats virtually impossible in typical applications.
