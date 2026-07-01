#include "MemoryPool.h"
#include <algorithm>

namespace FastECS {
namespace Memory {

    FixedSizeBlockAllocator::FixedSizeBlockAllocator(std::size_t blockSize, std::size_t blocksPerChunk)
        : m_blocksPerChunk(blocksPerChunk), m_freeListHead(nullptr) 
    {
        m_blockSize = std::max(blockSize, sizeof(void*));
        
        constexpr std::size_t alignment = alignof(std::max_align_t);
        m_blockSize = (m_blockSize + alignment - 1) & ~(alignment - 1);
        
        assert(m_blockSize >= sizeof(void*));
        assert(m_blocksPerChunk > 0);
    }

    FixedSizeBlockAllocator::~FixedSizeBlockAllocator() {
        clear();
    }

    void* FixedSizeBlockAllocator::allocate() {
        if (!m_freeListHead) {
            allocate_new_chunk();
        }

        void* result = m_freeListHead;
        m_freeListHead = *(static_cast<void**>(result));

        return result;
    }

    void FixedSizeBlockAllocator::deallocate(void* ptr) {
        if (!ptr) return;

        *(static_cast<void**>(ptr)) = m_freeListHead;
        m_freeListHead = ptr;
    }

    void FixedSizeBlockAllocator::clear() {
        m_chunks.clear();
        m_freeListHead = nullptr;
    }

    void FixedSizeBlockAllocator::allocate_new_chunk() {
        std::size_t chunkSize = m_blockSize * m_blocksPerChunk;
        Chunk newChunk;
        newChunk.data = std::make_unique<uint8_t[]>(chunkSize);

        uint8_t* chunkStart = newChunk.data.get();

        for (std::size_t i = 0; i < m_blocksPerChunk - 1; ++i) {
            uint8_t* currentBlock = chunkStart + i * m_blockSize;
            uint8_t* nextBlock = chunkStart + (i + 1) * m_blockSize;
            *(reinterpret_cast<void**>(currentBlock)) = nextBlock;
        }

        uint8_t* lastBlock = chunkStart + (m_blocksPerChunk - 1) * m_blockSize;
        *(reinterpret_cast<void**>(lastBlock)) = m_freeListHead;

        m_freeListHead = chunkStart;
        m_chunks.push_back(std::move(newChunk));
    }

}
}
