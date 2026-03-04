// Created my Jens Kromdijk 13-12-2025
#ifndef SPATIAL_HASHING_HPP
#define SPATIAL_HASHING_HPP

#include <iostream>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/ext/vector_uint3.hpp>

namespace SpatialHashing
{
    inline constexpr float CELL_SIZE{60.f};
    inline constexpr float CELL_PADDING{50.f};
    inline constexpr float WORLD_CHUNK_LIMIT{30.f}; // discard entities too far away from origin

    // pcg3d hash function for 3D coordinates
    // https://jcgt.org/published/0009/03/02/paper.pdf
    inline glm::uvec3 pcg3d(glm::uvec3 v)
    {
        v = v * 1664525u + 1013904223u;
        v.x += v.y * v.z;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        v ^= v >> 17u;
        v.x += v.y * v.z;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        return v;
    }

    struct ChunkKey
    {
        long long x{}, y{}, z{};
        bool operator==(const ChunkKey& other) const { return x == other.x && y == other.y && z == other.z; }
    };

    struct ChunkKeyHasher
    {
        std::size_t operator()(const ChunkKey& key) const
        {
            glm::uvec3 hashInput{static_cast<unsigned int>(key.x), static_cast<unsigned int>(key.y),
                                 static_cast<unsigned int>(key.z)};
            hashInput = pcg3d(hashInput);

            constexpr unsigned int prime1{73856093};
            constexpr unsigned int prime2{19349663};
            constexpr unsigned int prime3{83492791};

            return (static_cast<std::size_t>(hashInput.x) * prime1) ^ (static_cast<std::size_t>(hashInput.y) * prime2) ^
                (static_cast<std::size_t>(hashInput.z) * prime3);
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const ChunkKey& key)
    {
        os << key.x << ", " << key.y << ", " << key.z;
        return os;
    }
} // namespace SpatialHashing

#endif // SPATIAL_HASHING_HPP
