#pragma once
#include <vector>
#include <cstdint>

// Sparse set: a packed (dense) array of components for cache-friendly iteration,
// plus a sparse array mapping entity-index -> position in the dense array.
// O(1) add / remove (swap-and-pop) / get.
template <typename T>
class ComponentPool {
public:
    bool has(std::uint32_t idx) const {
        return idx < m_sparse.size()
            && m_sparse[idx] != NPOS
            && m_sparse[idx] < m_denseEntities.size()
            && m_denseEntities[m_sparse[idx]] == idx;
    }

    T& add(std::uint32_t idx, const T& value) {
        if (idx >= m_sparse.size()) m_sparse.resize(idx + 1, NPOS);
        if (has(idx)) { m_dense[m_sparse[idx]] = value; return m_dense[m_sparse[idx]]; }
        m_sparse[idx] = static_cast<std::uint32_t>(m_dense.size());
        m_dense.push_back(value);
        m_denseEntities.push_back(idx);
        return m_dense.back();
    }

    void remove(std::uint32_t idx) {
        if (!has(idx)) return;
        std::uint32_t pos     = m_sparse[idx];
        std::uint32_t lastPos = static_cast<std::uint32_t>(m_dense.size()) - 1;
        // swap the removed element with the last, then pop — keeps dense packed
        m_dense[pos]         = m_dense[lastPos];
        m_denseEntities[pos] = m_denseEntities[lastPos];
        m_sparse[m_denseEntities[pos]] = pos;
        m_dense.pop_back();
        m_denseEntities.pop_back();
        m_sparse[idx] = NPOS;
    }

    T* get(std::uint32_t idx) { return has(idx) ? &m_dense[m_sparse[idx]] : nullptr; }

    std::vector<T>&             data()     { return m_dense; }
    std::vector<std::uint32_t>& entities() { return m_denseEntities; }
    std::size_t size() const { return m_dense.size(); }

private:
    static constexpr std::uint32_t NPOS = 0xFFFFFFFFu;
    std::vector<std::uint32_t> m_sparse;         // entity index -> dense position
    std::vector<T>             m_dense;          // packed components
    std::vector<std::uint32_t> m_denseEntities;  // packed owning entity indices
};
