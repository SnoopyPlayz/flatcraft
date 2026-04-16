#include "map.hpp"
#include "rayUtils.hpp"
#include <iterator>
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
void dropItem(Block block, Vector3 pos){
	items.push_back(Item{block, pos});
}

void drawItems(){
	for (Item i : items) {
		Vector3 itemShadowPos = Vector3AddValue(i.pos, 3);
		itemShadowPos.y -= 10;

		const Texture2D itemTexture = useTexture(getEnumName(i.b) + ".png");
		const Vector3 itemPos = i.pos;

		float alpha = (itemShadowPos.y - player.pos.y * BLOCK_SIZE) * 0.001 + 1;
		const Color shadowTint = ColorAlpha(GRAY, alpha);
		const Color itemTint = ColorAlpha(WHITE, alpha);

		debug.addMessage(vector3ToString(itemPos));

		queueDraw3D(
			itemShadowPos.y,
			[itemTexture, itemShadowPos, shadowTint]() {
				DrawTextureWithRot(itemTexture, itemShadowPos.x, itemShadowPos.z, 0, shadowTint, 0.3f);
			}
		);
		queueDraw3D(
			itemPos.y,
			[itemTexture, itemPos, itemTint]() {
				DrawTextureWithRot(itemTexture, itemPos.x, itemPos.z, 0, itemTint, 1.0f);
			}
		);
	}
}

void pickUpItems(Vector3 pos){
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
