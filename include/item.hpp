#pragma once
#include "map.hpp"
#include "player.hpp"

struct Item{
	Block b;
	Vector3 pos;
};

void dropItem(Block block, Vector3 pos);
void pickUpItems(Vector3 pos = player.pos);
void drawItems();
