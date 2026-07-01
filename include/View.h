#pragma once

#include <tuple>
#include <utility>
#include "ComponentManager.h"
#include "SparseSet.h"

namespace FastECS {

template <typename... Components>
class View {
public:
    View(ComponentManager& manager) 
        : m_pools(manager.get_component_pool<Components>()...) 
    {
    }

    template <typename Func>
    void each(Func&& func) {
        auto* leadPool = std::get<0>(m_pools);
        const auto& denseToEntity = leadPool->get_dense_to_entity();

        for (size_t i = 0; i < denseToEntity.size(); ++i) {
            Entity e = denseToEntity[i];

            if (has_all_components(e, std::make_index_sequence<sizeof...(Components)>{})) {
                func(e, (std::get<SparseSet<Components>*>(m_pools)->get(e))...);
            }
        }
    }

private:
    std::tuple<SparseSet<Components>*...> m_pools;

    template <size_t... Is>
    bool has_all_components(Entity e, std::index_sequence<Is...>) {
        return (std::get<Is>(m_pools)->contains(e) && ...);
    }
};

}
