# FastECS

A high-performance, purely Data-Oriented Design (DOD) Entity-Component-System (ECS) engine implemented in C++20.

## Features

- **No RTTI & No Virtual Functions in Hot Paths**: Uses compile-time template instantiation for Component TypeIDs.
- **Cache-Friendly Sparse Set Architecture**: Guarantees perfectly contiguous memory layout for components in the Dense Array, maximizing CPU L1/L2 cache hardware prefetching.
- **Custom Memory Allocator**: A Chunk & Block based memory pool utilizing an intrusive Free-List, avoiding `std::vector` reallocation spikes and dangling pointers.
- **O(1) Operations**: Entity creation/destruction and Component add/remove/get are all O(1) complexity (using Swap-and-Pop).
- **Zero-Cost Abstractions**: Employs perfect forwarding to construct components in-place and C++17 fold expressions for lightning-fast multi-component filtering in Views.
- **Lock-Free/SpinLock Work-Stealing Job System**: Built-in multi-threading support avoiding OS context switch overhead and False Sharing.

## Build Instructions

Requires a compiler supporting C++20 (MSVC, GCC 10+, or Clang 11+).

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Usage Example

```cpp
#include "Registry.h"
#include "JobSystem.h"

struct Position { float x, y, z; };
struct Velocity { float vx, vy, vz; };

int main() {
    FastECS::Registry registry;
    registry.register_component<Position>();
    registry.register_component<Velocity>();

    FastECS::Entity e = registry.create();
    registry.emplace<Position>(e, 0.0f, 0.0f, 0.0f);
    registry.emplace<Velocity>(e, 1.0f, 1.0f, 1.0f);

    auto view = registry.view<Position, Velocity>();
    view.each([](FastECS::Entity e, Position& pos, Velocity& vel) {
        pos.x += vel.vx * 0.016f;
    });

    return 0;
}
```
