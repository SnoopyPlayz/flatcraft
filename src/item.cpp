#include "map.hpp"
#include "rayUtils.hpp"
#include <iterator>
#include <mutex>
#include <ostream>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include <vector>
#include <iostream>
#include "player.hpp"
#include "debug.hpp"
#include "vector.hpp"
#include "item.hpp"


std::vector<Item> items;
std::mutex itemMtx;
void dropItem(Block block, Vector3 pos){
	std::lock_guard<std::mutex> lock(itemMtx);
	items.push_back({block, pos});
}

void drawItems(){
	for (Item i : items) {
		Vector3 itemShadowPos = Vector3AddValue(i.pos, 3);
		itemShadowPos.y -= 10;
		float alpha = (itemShadowPos.y - player.pos.y * BLOCK_SIZE) * 0.001 + 1;
		debug.addMessage(std::to_string(alpha));

		drawTexture3D(useTexture(getEnumName(i.b) + ".png"), itemShadowPos, ColorAlpha(GRAY, alpha), 0, 0.3);
		drawTexture3D(useTexture(getEnumName(i.b) + ".png"), i.pos, ColorAlpha(WHITE, alpha), 0, 0.3);
	}
}

// default is player.pos
void pickUpItems(Vector3 pos){
	std::lock_guard<std::mutex> lock(itemMtx);
	for (unsigned long i {}; i < items.size(); i++) {
		Vector3 itemPos = items[i].pos;
		itemPos.x += BLOCK_SIZE / 2.;
		itemPos.z += BLOCK_SIZE / 2.;
		float distance = Vector3Distance(itemPos, pos * BLOCK_SIZE);
		if (distance < BLOCK_SIZE * 1.5) {
			std::swap(items[i], items.back());
			items.pop_back();
		}
	}
}
