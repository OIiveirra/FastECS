#pragma once

#include <queue>
#include <vector>
#include <cassert>
#include "Entity.h"

namespace FastECS {

class EntityManager {
public:
    EntityManager() : m_livingEntityCount(0) {}

    Entity create_entity() {
        assert(m_livingEntityCount < ENTITY_INDEX_MASK);

        uint32_t index;
        uint32_t version = 0;

        if (!m_freeIndices.empty()) {
            index = m_freeIndices.front();
            m_freeIndices.pop();
            version = m_versions[index];
        } else {
            index = static_cast<uint32_t>(m_versions.size());
            m_versions.push_back(0);
        }

        m_livingEntityCount++;
        return CreateEntityId(index, version);
    }

    void destroy_entity(Entity e) {
        uint32_t index = GetEntityIndex(e);
        
        m_versions[index]++;
        
        m_freeIndices.push(index);
        m_livingEntityCount--;
    }

    bool is_alive(Entity e) const {
        uint32_t index = GetEntityIndex(e);
        if (index >= m_versions.size()) return false;
        return GetEntityVersion(e) == m_versions[index];
    }

private:
    std::queue<uint32_t> m_freeIndices;
    std::vector<uint32_t> m_versions;
    uint32_t m_livingEntityCount;
};

}
