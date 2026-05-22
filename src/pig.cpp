#include "pig.hpp"
#include "player.hpp"
#include "rayUtils.hpp"
#include "map.hpp"
#include <algorithm>
#include <raylib.h>
#include <raymath.h>
#include <vector>

std::vector<Pig> pigs;

void drawPigs() {
	for (const Pig& p : pigs) {
		const float halfBlock = (float)BLOCK_SIZE / 2.0f;
		Vector3 pigCenter = {p.pos.x - halfBlock, p.pos.y, p.pos.z - halfBlock};
		Vector3 pigShadowPos = {pigCenter.x + 3, pigCenter.y - 10, pigCenter.z + 3};

		float alpha = (pigShadowPos.y - player.pos.y * BLOCK_SIZE) * 0.001f + 1.0f;
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		const Texture2D pigTexture = useTexture("pig.png");
		const Color shadowTint = ColorAlpha(GRAY, alpha);
		const Color pigTint = ColorAlpha(WHITE, alpha);

		queueDraw3D(pigShadowPos.y,
			[pigTexture, pigShadowPos, p, shadowTint]() {
				DrawTextureWithRot(pigTexture, pigShadowPos.x, pigShadowPos.z, p.rotation + 90, shadowTint, 1.0f);
			}
		);
		queueDraw3D(pigCenter.y,
			[pigTexture, pigCenter, p, pigTint]() {
				DrawTextureWithRot(pigTexture, pigCenter.x, pigCenter.z, p.rotation + 90, pigTint, 1.0f);
			}
		);
	}
}
