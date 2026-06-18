#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include "ecs/Entity.h"
#include "ecs/ComponentPool.h"

class Registry {
public:
    Entity create() {
        std::uint32_t idx;
        if (!m_free.empty()) { idx = m_free.back(); m_free.pop_back(); }
        else { idx = static_cast<std::uint32_t>(m_generations.size()); m_generations.push_back(0); }
        return makeEntity(idx, m_generations[idx]);
    }

    void destroy(Entity e) {
        std::uint32_t idx = entityIndex(e);
        if (idx >= m_generations.size()) return;
        for (auto& p : m_pools) if (p) p->removeIfPresent(idx);
        ++m_generations[idx];          // invalidate existing handles to this index
        m_free.push_back(idx);
    }

    bool valid(Entity e) const {
        std::uint32_t idx = entityIndex(e);
        return idx < m_generations.size() && m_generations[idx] == entityGeneration(e);
    }

    std::size_t aliveCount() const { return m_generations.size() - m_free.size(); }

    template <typename T> T&   add(Entity e, const T& c) { return pool<T>().add(entityIndex(e), c); }
    template <typename T> T*   get(Entity e)             { return pool<T>().get(entityIndex(e)); }
    template <typename T> bool has(Entity e)             { return pool<T>().has(entityIndex(e)); }
    template <typename T> void remove(Entity e)          { pool<T>().remove(entityIndex(e)); }

    // Call fn(entity, A&, Rest&...) for every entity that has ALL of A, Rest...
    // NOTE: don't add/remove components of the iterated types inside fn — that
    // mutates the dense array you're walking. Modifying VALUES is fine.
    template <typename A, typename... Rest, typename Fn>
    void view(Fn fn) {
        auto& a    = pool<A>();
        auto& ents = a.entities();
        for (std::size_t i = 0; i < ents.size(); ++i) {
            std::uint32_t idx = ents[i];
            if ((pool<Rest>().has(idx) && ...)) {
                fn(makeEntity(idx, m_generations[idx]), a.data()[i], *pool<Rest>().get(idx)...);
            }
        }
    }

private:
    struct IPool { virtual ~IPool() = default; virtual void removeIfPresent(std::uint32_t) = 0; };
    template <typename T>
    struct PoolHolder : IPool {
        ComponentPool<T> pool;
        void removeIfPresent(std::uint32_t idx) override { pool.remove(idx); }
    };

    static std::uint32_t nextTypeId() { static std::uint32_t id = 0; return id++; }
    template <typename T> static std::uint32_t typeId() { static std::uint32_t id = nextTypeId(); return id; }

    template <typename T>
    ComponentPool<T>& pool() {
        std::uint32_t id = typeId<T>();
        if (id >= m_pools.size()) m_pools.resize(id + 1);
        if (!m_pools[id]) m_pools[id] = std::make_unique<PoolHolder<T>>();
        return static_cast<PoolHolder<T>*>(m_pools[id].get())->pool;
    }

    std::vector<std::uint32_t>          m_generations;
    std::vector<std::uint32_t>          m_free;
    std::vector<std::unique_ptr<IPool>> m_pools;
};
