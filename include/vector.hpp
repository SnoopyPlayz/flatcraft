#pragma once

#include <cstdint>
#include <raylib.h>
#include <iostream>

namespace math {
    int mod(int a, int b);
}

struct Vec3Int {
    int32_t x, y, z;

    bool operator<(const Vec3Int& other) const;
    Vec3Int operator/(const float c) const;
    Vec3Int operator*(const int) const;
    Vec3Int operator*(const Vec3Int& other) const;
    Vec3Int operator-(const Vec3Int& other) const;
    Vec3Int operator+(const Vec3Int& other) const;
    Vec3Int operator-(const int other) const;
    bool operator==(const Vec3Int& other) const;
    Vec3Int mod(int a) const;
    Vector3 toVec3() const;
};

Vec3Int toVec3Int(const Vector3& v);

std::string vector3ToString(auto vec){
	return "%R" + std::to_string(vec.x) + "%G " + std::to_string(vec.y) + "%B " + std::to_string(vec.z);
}
