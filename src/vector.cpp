#include "vector.hpp"
#include <cassert>
#include <raylib.h>
#include <cmath>
#include <iostream>
#include "vector.hpp"

namespace math{
	int mod(int a, int b){
		int r = std::div((int)a, (int)b).rem;
		r += (-(r < 0) & b);
		return r;
	}
}

bool Vec3Int::operator<(const Vec3Int& other) const {
	if (x != other.x)
		return x < other.x;

	if (y != other.y)
		return y < other.y;

	return z < other.z;
}

Vec3Int Vec3Int::operator*(const Vec3Int& other) const{
	return {x * other.x, y * other.y, z * other.z};
}

Vec3Int Vec3Int::operator+(const Vec3Int& other) const{
	return {x + other.x, y + other.y, z + other.z};
}

Vec3Int Vec3Int::operator-(const Vec3Int& other) const{
	return {x - other.x, y - other.y, z - other.z};
}

Vec3Int Vec3Int::operator*(const int other) const{
	return {x * other, y * other, z * other};
}

Vec3Int Vec3Int::operator-(const int other) const{
	return {x - other, y - other, z - other};
}

bool Vec3Int::operator==(const Vec3Int& other) const{
	return (x == other.x && y == other.y && z == other.z);
}

Vec3Int Vec3Int::operator/(const float c) const{
	Vec3Int ret;
	ret.x = std::floor((float)x / (float)c);
	ret.y = std::floor((float)y / (float)c);
	ret.z = std::floor((float)z / (float)c);
	return ret;
}

Vec3Int Vec3Int::mod(int a) const{
	return {math::mod(this->x, a), math::mod(this->y, a), math::mod(this->z, a)};
}

Vector3 Vec3Int::toVec3() const{
	return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
}

Vec3Int toVec3Int(const Vector3& v){
	return {static_cast<int>(std::floor(v.x)), static_cast<int>(std::floor(v.y)), static_cast<int>(std::floor(v.z))};
}
