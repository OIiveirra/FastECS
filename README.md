# FastECS

一个基于纯 C++20 实现的、高性能、数据驱动（Data-Oriented Design, DOD）的实体-组件-系统（Entity-Component-System, ECS）底层架构引擎。

## 核心特性

- **无 RTTI 与零虚函数开销**：利用 C++ 模板实例化在编译期生成组件的 TypeID，彻底抛弃运行时的多态开销。
- **缓存友好 (Cache-Friendly) 的稀疏集架构**：底层采用 Sparse Set 结构，保证组件数据在稠密数组 (Dense Array) 中的绝对物理连续性，最大化榨干 CPU L1/L2 硬件预取性能。
- **自定义内存分配器**：实现基于 Chunk 与 Block 的连续内存池，配合侵入式空闲链表 (Intrusive Free-List)，彻底解决 `std::vector` 扩容导致的掉帧、数据拷贝与幽灵指针问题。
- **极致的 O(1) 性能**：实体的创建与销毁、组件的增删查改全部为 O(1) 复杂度（利用 Swap-and-Pop 技法）。
- **零成本抽象**：使用完美转发（Perfect Forwarding）实现组件的就地构造，结合 C++17 折叠表达式实现闪电般的多组件 View 视图过滤。
- **工作窃取线程池 (Work-Stealing Job System)**：内置轻量级自旋锁 (SpinLock) 与任务窃取调度系统，有效避免 OS 线程上下文切换开销与 False Sharing（伪共享）问题，实现无缝多线程并发计算。

## 编译环境要求

需要支持 C++20 标准的编译器（如 MSVC、GCC 10+、或 Clang 11+）。

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 快速使用示例

```cpp
#include "Registry.h"
#include "JobSystem.h"

struct Position { float x, y, z; };
struct Velocity { float vx, vy, vz; };

int main() {
    FastECS::Registry registry;
    
    // 1. 注册组件
    registry.register_component<Position>();
    registry.register_component<Velocity>();

    // 2. 创建实体并就地挂载组件
    FastECS::Entity e = registry.create();
    registry.emplace<Position>(e, 0.0f, 0.0f, 0.0f);
    registry.emplace<Velocity>(e, 1.0f, 1.0f, 1.0f);

    // 3. 创建视图并过滤出同时拥有 Position 和 Velocity 的实体进行逻辑迭代
    auto view = registry.view<Position, Velocity>();
    view.each([](FastECS::Entity e, Position& pos, Velocity& vel) {
        pos.x += vel.vx * 0.016f;
    });

    return 0;
}
```
