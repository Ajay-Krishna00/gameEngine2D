#pragma once
#include <cstdint>

// Entity = 32-bit index + 32-bit generation, packed into 64 bits.
// The generation invalidates stale handles after an index is recycled.
using Entity = std::uint64_t;
constexpr Entity NULL_ENTITY = 0xFFFFFFFFFFFFFFFFull;

inline std::uint32_t entityIndex(Entity e)      { return static_cast<std::uint32_t>(e & 0xFFFFFFFFull); }
inline std::uint32_t entityGeneration(Entity e) { return static_cast<std::uint32_t>(e >> 32); }
inline Entity makeEntity(std::uint32_t idx, std::uint32_t gen) {
    return (static_cast<Entity>(gen) << 32) | static_cast<Entity>(idx);
}
