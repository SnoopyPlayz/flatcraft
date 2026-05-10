#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>
#include "map.hpp"

using CraftingKey = std::array<uint8_t, 9>;

struct CraftingKeyHash {
	size_t operator()(const CraftingKey& k) const {
		size_t h = 0;
		for (int i = 0; i < 9; i++)
			h ^= std::hash<uint8_t>{}(k[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
		return h;
	}
};

struct CraftingKeyEqual {
	bool operator()(const CraftingKey& a, const CraftingKey& b) const {
		return a == b;
	}
};

void initRecipes();
Block lookupRecipe(const uint8_t slots[9]);
void normalizeGrid(const uint8_t input[9], uint8_t output[9]);
