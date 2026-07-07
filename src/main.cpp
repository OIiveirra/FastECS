#include <iostream>
#include "MemoryPool.h"
#include "Entity.h"
#include "Registry.h"
#include "JobSystem.h"
#include <chrono>

struct Position {
    float x, y, z;
};

struct Velocity {
    float vx, vy, vz;
};

struct Renderable {
    int meshId;
};

// 单线程物理更新系统
void PhysicsSystem_SingleThread(FastECS::Registry& registry, float dt) {
    auto view = registry.view<Position, Velocity>();
    // 遍历同时拥有 Position 和 Velocity 的实体
    view.each([dt](FastECS::Entity, Position& pos, Velocity& vel) {
        pos.x += vel.vx * dt;
        pos.y += vel.vy * dt;
        pos.z += vel.vz * dt;
    });
}

// 多线程并发物理更新系统
void PhysicsSystem_MultiThread(FastECS::Registry& registry, FastECS::JobSystem& jobSystem, float dt) {
    auto* posPool = registry.get_pool<Position>();
    auto* velPool = registry.get_pool<Velocity>();

    const auto& entities = posPool->get_dense_to_entity();
    size_t totalEntities = entities.size();
    if (totalEntities == 0) return;

    // 将连续数组分块（Batching），每块 10000 个实体。
    // 核心目的：让不同线程操作物理内存相隔极远的数据块，彻底规避伪共享 (False Sharing)！
    const size_t CHUNK_SIZE = 10000;
    size_t numChunks = (totalEntities + CHUNK_SIZE - 1) / CHUNK_SIZE;

    for (size_t chunk = 0; chunk < numChunks; ++chunk) {
        size_t startIdx = chunk * CHUNK_SIZE;
        size_t endIdx = std::min(startIdx + CHUNK_SIZE, totalEntities);

        // 将每个数据块作为一个 Job 提交给线程池
        jobSystem.execute([startIdx, endIdx, posPool, velPool, &entities, dt]() {
            for (size_t i = startIdx; i < endIdx; ++i) {
                FastECS::Entity e = entities[i];
                if (velPool->contains(e)) {
                    auto& pos = posPool->get(e);
                    auto& vel = velPool->get(e);
                    pos.x += vel.vx * dt;
                    pos.y += vel.vy * dt;
                    pos.z += vel.vz * dt;
                }
            }
        });
    }

    // 主线程等待本帧所有物理更新任务完成
    jobSystem.wait();
}

int main() {
    std::cout << "FastECS Engine Initialization" << std::endl;

    FastECS::JobSystem jobSystem;
    jobSystem.init();

    FastECS::Registry registry;

    registry.register_component<Position>();
    registry.register_component<Velocity>();
    registry.register_component<Renderable>();

    // 创建 100 万个实体用于压测
    const int ENTITY_COUNT = 1000000;
    
    for (int i = 0; i < ENTITY_COUNT; ++i) {
        FastECS::Entity e = registry.create();
        registry.emplace<Position>(e, 0.0f, 0.0f, 0.0f);
        
        // 50 万个实体有 Velocity
        if (i % 2 == 0) {
            registry.emplace<Velocity>(e, 1.0f, 1.0f, 1.0f);
        }

        // 10 万个实体有 Renderable
        if (i % 10 == 0) {
            registry.emplace<Renderable>(e, 999);
        }
    }

    auto startSingle = std::chrono::high_resolution_clock::now();
    PhysicsSystem_SingleThread(registry, 0.016f);
    auto endSingle = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diffSingle = endSingle - startSingle;
    std::cout << "Single-Thread execution: " << diffSingle.count() << " ms" << std::endl;

    auto startMulti = std::chrono::high_resolution_clock::now();
    PhysicsSystem_MultiThread(registry, jobSystem, 0.016f);
    auto endMulti = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diffMulti = endMulti - startMulti;
    std::cout << "Multi-Thread execution: " << diffMulti.count() << " ms" << std::endl;

    std::cout << "Speedup: " << diffSingle.count() / diffMulti.count() << "x" << std::endl;

    std::cout << "\n----------------------------------------\n";
    std::cout << "Engine test completed. Press any key to exit." << std::endl;
    
    // 在 Windows 系统下，用 system("pause") 是防止双击 exe 闪退的最稳定方法。
    system("pause");

    return 0;
}
