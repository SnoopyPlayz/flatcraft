#pragma once
#include <cstdint>
#include <vector>
#include "map.hpp"
#include "player.hpp"

struct Item{
	uint32_t id;
	Block b;
	Vector3 pos;
	Vector3 velocity;
};

extern std::vector<Item> items;
extern std::vector<uint32_t> pickedItemIds;
void pickUpItems(Vector3 pos = player.pos);
void drawItems();
