#include "crafting.hpp"
#include "map.hpp"
#include <algorithm>
#include <cstring>

static std::unordered_map<CraftingKey, Block, CraftingKeyHash, CraftingKeyEqual> recipes;

// move all the items to top left for hasing
void normalizeGrid(const uint8_t input[9], uint8_t output[9]) {
	std::fill(output, output + 9, (uint8_t)AIR);

	int minRow = 3, minCol = 3;
	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			if (input[r * 3 + c] != AIR) {
				minRow = std::min(minRow, r);
				minCol = std::min(minCol, c);
			}
		}
	}

	if (minRow == 3) return; // all AIR

	for (int r = minRow; r < 3; r++) {
		for (int c = minCol; c < 3; c++) {
			int src = r * 3 + c;
			int dst = (r - minRow) * 3 + (c - minCol);
			output[dst] = input[src];
		}
	}
}

void initRecipes() {
	// 2x2 STONE -> CRAFTING_TABLE (test recipe)
	CraftingKey key;
	key.fill((uint8_t)AIR);
	key[0] = STONE; key[1] = STONE;
	key[3] = STONE; key[4] = STONE;
	recipes[key] = CRAFTING_TABLE;
}

Block lookupRecipe(const uint8_t slots[9]) {
	uint8_t normalized[9];
	normalizeGrid(slots, normalized);

	CraftingKey key;
	memcpy(key.data(), normalized, 9);

	auto it = recipes.find(key);
	if (it != recipes.end()) return it->second;
	return AIR;
}
