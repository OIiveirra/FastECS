#pragma once

#include <memory>
#include "Entity.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "View.h"

namespace FastECS {

class Registry {
public:
    Registry() 
        : m_entityManager(std::make_unique<EntityManager>())
        , m_componentManager(std::make_unique<ComponentManager>()) 
    {}

    Entity create() {
        return m_entityManager->create_entity();
    }

    void destroy(Entity e) {
        if (m_entityManager->is_alive(e)) {
            m_componentManager->entity_destroyed(e);
            m_entityManager->destroy_entity(e);
        }
    }

    bool alive(Entity e) const {
        return m_entityManager->is_alive(e);
    }
    
    template <typename T>
    void register_component() {
        m_componentManager->init_component_pool<T>();
    }

    template <typename T, typename... Args>
    T& emplace(Entity e, Args&&... args) {
        assert(alive(e));
        return m_componentManager->add_component<T>(e, std::forward<Args>(args)...);
    }

    template <typename T>
    void remove(Entity e) {
        assert(alive(e));
        m_componentManager->remove_component<T>(e);
    }

    template <typename T>
    T& get(Entity e) {
        assert(alive(e));
        return m_componentManager->get_component<T>(e);
    }

    template <typename T>
    SparseSet<T>* get_pool() {
        return m_componentManager->get_component_pool<T>();
    }

    template <typename T>
    bool has(Entity e) {
        if (!alive(e)) return false;
        return m_componentManager->has_component<T>(e);
    }

    template <typename... Components>
    View<Components...> view() {
        return View<Components...>(*m_componentManager);
    }

private:
    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<ComponentManager> m_componentManager;
};

}
