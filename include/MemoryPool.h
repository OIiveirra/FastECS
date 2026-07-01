#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include <cassert>

namespace FastECS {
namespace Memory {

    class FixedSizeBlockAllocator {
    public:
        FixedSizeBlockAllocator(std::size_t blockSize, std::size_t blocksPerChunk);
        ~FixedSizeBlockAllocator();

        FixedSizeBlockAllocator(const FixedSizeBlockAllocator&) = delete;
        FixedSizeBlockAllocator& operator=(const FixedSizeBlockAllocator&) = delete;
        FixedSizeBlockAllocator(FixedSizeBlockAllocator&&) noexcept = default;
        FixedSizeBlockAllocator& operator=(FixedSizeBlockAllocator&&) noexcept = default;

        void* allocate();
        void deallocate(void* ptr);
        void clear();

    private:
        void allocate_new_chunk();

    private:
        std::size_t m_blockSize;
        std::size_t m_blocksPerChunk;
        
        struct Chunk {
            std::unique_ptr<uint8_t[]> data;
        };
        
        std::vector<Chunk> m_chunks;
        void* m_freeListHead = nullptr;
    };

}
}
