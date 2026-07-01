#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <cassert>
#include "Entity.h"
#include "SparseSet.h"

namespace FastECS {

    using ComponentTypeID = uint32_t;

    inline ComponentTypeID get_unique_component_id() {
        static ComponentTypeID counter = 0;
        return counter++;
    }

    template <typename T>
    inline ComponentTypeID GetComponentTypeID() {
        static const ComponentTypeID id = get_unique_component_id();
        return id;
    }

    class ComponentManager {
    public:
        template <typename T>
        void init_component_pool() {
            ComponentTypeID id = GetComponentTypeID<T>();
            if (id >= m_componentPools.size()) {
                m_componentPools.resize(id + 1, nullptr);
            }
            if (!m_componentPools[id]) {
                m_componentPools[id] = std::make_shared<SparseSet<T>>();
            }
        }

        template <typename T>
        SparseSet<T>* get_component_pool() {
            ComponentTypeID id = GetComponentTypeID<T>();
            assert(id < m_componentPools.size() && m_componentPools[id] != nullptr);
            return static_cast<SparseSet<T>*>(m_componentPools[id].get());
        }

        template <typename T, typename... Args>
        T& add_component(Entity e, Args&&... args) {
            return get_component_pool<T>()->emplace(e, std::forward<Args>(args)...);
        }

        template <typename T>
        T& get_component(Entity e) {
            return get_component_pool<T>()->get(e);
        }

        template <typename T>
        void remove_component(Entity e) {
            get_component_pool<T>()->remove(e);
        }

        template <typename T>
        bool has_component(Entity e) {
            return get_component_pool<T>()->contains(e);
        }

        void entity_destroyed(Entity e) {
            for (auto const& pool : m_componentPools) {
                if (pool) {
                    pool->entityDestroyed(e);
                }
            }
        }

    private:
        std::vector<std::shared_ptr<ISparseSet>> m_componentPools;
    };

}
