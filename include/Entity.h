#pragma once
#include <cstdint>

namespace FastECS {

    using Entity = uint32_t;

    const uint32_t ENTITY_INDEX_BITS = 20;
    const uint32_t ENTITY_INDEX_MASK = (1 << ENTITY_INDEX_BITS) - 1;
    const uint32_t ENTITY_VERSION_MASK = ~ENTITY_INDEX_MASK;

    inline uint32_t GetEntityIndex(Entity e) {
        return e & ENTITY_INDEX_MASK;
    }

    inline uint32_t GetEntityVersion(Entity e) {
        return (e & ENTITY_VERSION_MASK) >> ENTITY_INDEX_BITS;
    }

    inline Entity CreateEntityId(uint32_t index, uint32_t version) {
        return ((version << ENTITY_INDEX_BITS) & ENTITY_VERSION_MASK) | (index & ENTITY_INDEX_MASK);
    }
    
    const Entity NULL_ENTITY = 0xFFFFFFFF;

}
