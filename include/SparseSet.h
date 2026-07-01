#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include "Entity.h"

namespace FastECS {

class ISparseSet {
public:
    virtual ~ISparseSet() = default;
    virtual void entityDestroyed(Entity e) = 0;
};

template <typename T>
class SparseSet : public ISparseSet {
public:
    SparseSet() = default;
    
    void entityDestroyed(Entity e) override {
        remove(e);
    }

    bool contains(Entity e) const {
        uint32_t index = GetEntityIndex(e);
        return index < m_sparse.size() && m_sparse[index] != NULL_ENTITY;
    }

    template <typename... Args>
    T& emplace(Entity e, Args&&... args) {
        uint32_t index = GetEntityIndex(e);
        
        if (index >= m_sparse.size()) {
            m_sparse.resize(index + 1, NULL_ENTITY);
        }

        if (!contains(e)) {
            m_sparse[index] = static_cast<uint32_t>(m_dense.size());
            m_dense.emplace_back(std::forward<Args>(args)...);
            m_denseToEntity.push_back(e);
        } else {
            m_dense[m_sparse[index]] = T{std::forward<Args>(args)...};
        }
        return m_dense[m_sparse[index]];
    }

    void insert(Entity e, const T& component) {
        emplace(e, component);
    }

    void remove(Entity e) {
        if (!contains(e)) return;

        uint32_t index = GetEntityIndex(e);
        uint32_t denseIndex = m_sparse[index];

        uint32_t lastDenseIndex = static_cast<uint32_t>(m_dense.size() - 1);
        Entity lastEntity = m_denseToEntity[lastDenseIndex];

        if (denseIndex != lastDenseIndex) {
            m_dense[denseIndex] = std::move(m_dense[lastDenseIndex]);
            m_denseToEntity[denseIndex] = lastEntity;
            m_sparse[GetEntityIndex(lastEntity)] = denseIndex;
        }

        m_dense.pop_back();
        m_denseToEntity.pop_back();
        m_sparse[index] = NULL_ENTITY;
    }

    T& get(Entity e) {
        assert(contains(e));
        return m_dense[m_sparse[GetEntityIndex(e)]];
    }

    const std::vector<T>& data() const { return m_dense; }
    std::vector<T>& data() { return m_dense; }
    const std::vector<Entity>& get_dense_to_entity() const { return m_denseToEntity; }

private:
    std::vector<uint32_t> m_sparse;
    std::vector<T> m_dense;
    std::vector<Entity> m_denseToEntity;
};

}
