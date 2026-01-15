#pragma once

#include <cstdint>

namespace math {
    int mod(int a, int b);
}

struct Vec3Int {
    int32_t x, y, z;

    bool operator<(const Vec3Int& other) const;
    Vec3Int operator/(const float c) const;
    Vec3Int mod(int a) const;
};
