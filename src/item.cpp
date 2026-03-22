#include "map.hpp"
#include "rayUtils.hpp"
#include <raymath.h>
#include <vector>
#include <iostream>

struct Item{
	Block b;
	Vector3 pos;
};

std::vector<Item> items;
void dropItem(Block block, Vector3 pos){
	items.push_back({block, pos});
}

void drawItems(){
	for (Item i : items) {
		Vector3 itemShadowPos = Vector3AddValue(i.pos, 3);
		itemShadowPos.y -= 10;

		drawTexture3DRot(useTexture(getEnumName(i.b) + ".png"), itemShadowPos, GRAY, 0, 0.3);
		drawTexture3DRot(useTexture(getEnumName(i.b) + ".png"), i.pos, WHITE, 0, 0.3);
	}
}
