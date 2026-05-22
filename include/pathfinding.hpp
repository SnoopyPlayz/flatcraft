#pragma once
#include <vector>
#include "vector.hpp"
#include "map.hpp"

std::vector<Vec3Int> findPath(Map& map, Vec3Int start, Vec3Int goal);
