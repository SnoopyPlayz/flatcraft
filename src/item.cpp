#include "map.hpp"
#include "rayUtils.hpp"
#include <algorithm>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "player.hpp"
#include "item.hpp"


std::vector<Item> items;
std::vector<uint32_t> pickedItemIds;

void drawItems(){
	for (Item i : items) {
		Vector3 itemShadowPos = {i.pos.x + 3, i.pos.y - 10, i.pos.z + 3};

		float alpha = (itemShadowPos.y - player.pos.y * BLOCK_SIZE) * 0.001f + 1.0f;
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		const Texture2D itemTexture = useTexture(getEnumName(i.b) + ".png");
		const Color shadowTint = ColorAlpha(GRAY, alpha);
		const Vector3 itemPos = i.pos;
		const Color itemTint = ColorAlpha(WHITE, alpha);

		queueDraw3D(itemShadowPos.y,
			[itemTexture, itemShadowPos, shadowTint]() {
				DrawTextureWithRot(itemTexture, itemShadowPos.x, itemShadowPos.z, 0, shadowTint, 0.3f);
			}
		);
		queueDraw3D(itemPos.y,
			[itemTexture, itemPos, itemTint]() {
				DrawTextureWithRot(itemTexture, itemPos.x, itemPos.z, 0, itemTint, 0.3f);
			}
		);
	}
}
