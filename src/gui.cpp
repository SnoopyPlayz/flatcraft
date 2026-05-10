#include "player.hpp"
#include "physics.hpp"
#include "rayUtils.hpp"

bool inventoryOpen = false;

void Player::inventoryUpdate() {
	const float windowBorder = 50;
	const float windowRoundess = 0.1;
	const int rowSize = 10;

	if (IsKeyPressed(KEY_E)) {
		inventoryOpen = !inventoryOpen;
	}
	if (!inventoryOpen) {
		return;
	}

	DrawRectangleRounded((Rectangle){windowBorder, windowBorder,
			(float)GetScreenWidth() - windowBorder * 2,
			(float)GetScreenHeight() - windowBorder * 2},
			windowRoundess, 0, WHITE);

	int y = 100;
	int x = 1;
	static bool selected = false;
	static Block selectedBlock = AIR;
	static int lastPos;

	for (int i{}; i < PLAYER_INVENTORY_SIZE; i++) {
		float pixelPosX = 1.2 * BLOCK_SIZE * x;

		if (x > rowSize || pixelPosX + BLOCK_SIZE > GetScreenWidth()) {
			x = 1;
			y += 100;
		}

		pixelPosX = 1.2 * BLOCK_SIZE * x;

		// item background
		const float itemGridOffset = 5;
		DrawRectangleRounded((Rectangle){pixelPosX - itemGridOffset,
				(float)y - itemGridOffset,
				BLOCK_SIZE + itemGridOffset * 2,
				BLOCK_SIZE + itemGridOffset * 2},
				windowRoundess, 1, DARKGRAY);

		x++;

		// draw item Tex
		if (inventory[i] != AIR) {
			DrawTexture(useTexture(getEnumName((Block)inventory[i]) + ".png"),
					pixelPosX, y, WHITE);
		}

		// colision with cursor
		if (AABBColBox2d(pixelPosX, y, BLOCK_SIZE, GetMouseX(), GetMouseY(), 1)) {
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inventory[i] != AIR) {
				selected = true;
				selectedBlock = (Block)inventory[i];
				inventory[i] = AIR;
				lastPos = i;
			}

			if (selected) {
				if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
					inventory[lastPos] = (Block)inventory[i];
					inventory[i] = selectedBlock;
					selected = false;
					selectedBlock = AIR;
					inventoryMoves.push_back({(uint8_t)lastPos, (uint8_t)i});
				}
			}

			DrawRectangle(pixelPosX, y, BLOCK_SIZE, BLOCK_SIZE,
					ColorAlpha(GRAY, 0.5));
		}

		// draw selectedBlock
		if (selected) {
			DrawTexture(useTexture(getEnumName(selectedBlock) + ".png"),
					GetMouseX() - BLOCK_SIZE * 0.5,
					GetMouseY() - BLOCK_SIZE * 0.5, WHITE);
		}
	}
}

void Player::updateUI() { inventoryUpdate(); }
