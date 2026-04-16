#pragma once
#include "map.hpp"
#include "player.hpp"

class Item{
public:
	Block b;
	Vector3 pos;
};

void dropItem(Block block, Vector3 pos);
void pickUpItems(Vector3 pos);
void drawItems();
