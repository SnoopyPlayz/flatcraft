#include "player.hpp"
#include "physics.hpp"
#include "rayUtils.hpp"

bool inventoryOpen = false;

struct DragState {
	bool active = false;
	Block block = AIR;
	int sourceSlot = -1;
	uint8_t* sourceArray = nullptr;
};
static DragState drag;

// draw one inventory slot cell, returns its screen rectangle
static Rectangle drawSlot(float x, float y, Block block, bool hovered) {
	const float itemGridOffset = 5;
	const float windowRoundess = 0.1f;

	DrawRectangleRounded(
		(Rectangle){x - itemGridOffset, y - itemGridOffset,
			BLOCK_SIZE + itemGridOffset * 2, BLOCK_SIZE + itemGridOffset * 2},
		windowRoundess, 1, DARKGRAY);

	if (block != AIR) {
		DrawTexture(useTexture(getEnumName(block) + ".png"), x, y, WHITE);
	}

	if (hovered) {
		DrawRectangle(x, y, BLOCK_SIZE, BLOCK_SIZE, ColorAlpha(GRAY, 0.5f));
	}

	return (Rectangle){x, y, BLOCK_SIZE, BLOCK_SIZE};
}

// highlight slot border in yellow (for hotbar selection indicator)
static void drawSlotHighlight(float x, float y) {
	const float offset = 5;
	DrawRectangleRoundedLines(
		(Rectangle){x - offset, y - offset,
			BLOCK_SIZE + offset * 2, BLOCK_SIZE + offset * 2},
		0.1f, 1, YELLOW);
}

// draw grid of slots from a uint8_t array, returns rects for hit testing
static std::vector<Rectangle> drawSlotGrid(uint8_t* slots, int slotCount, int cols, float startX, float startY, float spacing) {
	std::vector<Rectangle> rects;
	rects.reserve(slotCount);

	float x = startX;
	float y = startY;

	for (int i = 0; i < slotCount; i++) {
		if (i > 0 && i % cols == 0) {
			x = startX;
			y += BLOCK_SIZE + spacing;
		}

		Block block = (Block)slots[i];
		bool hovered = AABBColBox2d(x, y, BLOCK_SIZE, GetMouseX(), GetMouseY(), 1);

		drawSlot(x, y, block, hovered);
		rects.push_back({x, y, BLOCK_SIZE, BLOCK_SIZE});

		x += BLOCK_SIZE + spacing;
	}

	// draw dragged block at cursor
	if (drag.active) {
		DrawTexture(useTexture(getEnumName(drag.block) + ".png"),
			GetMouseX() - BLOCK_SIZE * 0.5f,
			GetMouseY() - BLOCK_SIZE * 0.5f, WHITE);
	}

	return rects;
}

// handle drag-and-drop interaction on a grid
static void handleGridDrag(uint8_t* slots, int slotCount,
		const std::vector<Rectangle>& rects) {
	for (int i = 0; i < slotCount; i++) {
		if (!AABBColBox2d(rects[i].x, rects[i].y, BLOCK_SIZE,
				GetMouseX(), GetMouseY(), 1))
			continue;

		// pickup
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !drag.active && slots[i] != AIR) {
			drag.active = true;
			drag.block = (Block)slots[i];
			drag.sourceSlot = i;
			drag.sourceArray = slots;
			slots[i] = AIR;
			break;
		}

		// drop / swap
		if (drag.active && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			if (drag.sourceArray == nullptr) {
				// drag from crafting output
				if (slots[i] == AIR) {
					slots[i] = (uint8_t)drag.block;
					for (int j = 0; j < CRAFTING_GRID_SIZE; j++)
						player.craftingSlots[j] = AIR;
					uint8_t toOffset = (slots == player.craftingSlots) ? SLOT_CRAFT_OFFSET : 0;
					inventoryMoves.push_back({SLOT_CRAFT_OUTPUT, (uint8_t)(toOffset + i)});
				} else {
					player.craftingResult = drag.block;
				}
			} else if (drag.sourceArray == slots && drag.sourceSlot == i) {
				slots[i] = drag.block; // restore to same slot
			} else {
				Block temp = (Block)slots[i];
				slots[i] = drag.block;
				if (drag.sourceArray) {
					drag.sourceArray[drag.sourceSlot] = temp;
				}

				uint8_t fromOffset = (drag.sourceArray == player.craftingSlots) ? SLOT_CRAFT_OFFSET : 0;
				uint8_t toOffset = (slots == player.craftingSlots) ? SLOT_CRAFT_OFFSET : 0;
				inventoryMoves.push_back(
					{(uint8_t)(fromOffset + drag.sourceSlot), (uint8_t)(toOffset + i)});
			}

			drag.active = false;
			drag.block = AIR;
			drag.sourceSlot = -1;
			drag.sourceArray = nullptr;
			break;
		}
	}
}

// cancel drag if mouse released outside all grids
static void cancelDragIfReleased() {
	if (drag.active && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		if (drag.sourceArray) {
			drag.sourceArray[drag.sourceSlot] = drag.block;
		} else {
			player.craftingResult = drag.block;
		}
		drag.active = false;
		drag.block = AIR;
		drag.sourceSlot = -1;
		drag.sourceArray = nullptr;
	}
}

// draw hearts above hotbar
static void drawHearts(float hotbarStartX, float hotbarStartY) {
	const float heartScale = 0.5f;
	const float heartSize = BLOCK_SIZE * heartScale;
	const float spacing = 4;
	float startX = hotbarStartX;
	float startY = hotbarStartY - heartSize - 8;

	const Texture2D heartTex = useTexture("hart.png");

	for (int i = 0; i < 10; i++) {
		Color tint = (i < player.health) ? WHITE : ColorAlpha(GRAY, 0.3f);
		Rectangle dest = {startX + i * (heartSize + spacing), startY, heartSize, heartSize};
		DrawTexturePro(heartTex, (Rectangle){0, 0, (float)heartTex.width, (float)heartTex.height},
			dest, (Vector2){0, 0}, 0, tint);
	}
}

// hotbar: first 10 slots, centered at bottom of screen
static void drawHotbar() {
	int screenW = GetScreenWidth();
	int screenH = GetScreenHeight();
	const float spacing = 10;

	float totalWidth = 10 * BLOCK_SIZE + 9 * spacing;
	float startX = (screenW - totalWidth) / 2.0f;
	float startY = screenH - BLOCK_SIZE - 12;

	drawHearts(startX, startY);

	auto rects = drawSlotGrid(player.inventory, 10, 10, startX, startY, spacing);
	handleGridDrag(player.inventory, 10, rects);

	// highlight selected hotbar slot
	drawSlotHighlight(rects[player.selectedSlot].x, rects[player.selectedSlot].y);

	cancelDragIfReleased();
}

// 3x3 crafting grid + output slot
static void drawCraftingGrid(float craftStartX, float craftStartY, float spacing) {
	// 3x3 input grid
	auto craftRects = drawSlotGrid(player.craftingSlots, 9, 3,
		craftStartX, craftStartY, spacing);
	handleGridDrag(player.craftingSlots, 9, craftRects);

	// output slot (to the right of the 3x3, vertically centered)
	float outputX = craftStartX + 3 * (BLOCK_SIZE + spacing) + 25;
	float outputY = craftStartY + BLOCK_SIZE + spacing * 0.5f;

	bool outputHovered = AABBColBox2d(outputX, outputY, BLOCK_SIZE,
		GetMouseX(), GetMouseY(), 1);
	drawSlot(outputX, outputY, player.craftingResult, outputHovered);

	// pickup from output slot (drag or click)
	if (outputHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
			player.craftingResult != AIR && !drag.active) {
		drag.active = true;
		drag.block = player.craftingResult;
		drag.sourceSlot = SLOT_CRAFT_OUTPUT;
		drag.sourceArray = nullptr;
		player.craftingResult = AIR;
	}

	// arrow icon hint between grid and output
	const char* arrowText = ">";
	int arrowFontSize = 30;
	float arrowX = craftStartX + 3 * (BLOCK_SIZE + spacing) + 5;
	float arrowY = outputY + BLOCK_SIZE / 2 - arrowFontSize / 2;
	drawTextSDF(arrowText, arrowX, arrowY, arrowFontSize, DARKGRAY);
}

void Player::inventoryUpdate() {
	const float windowBorder = 50;
	const float windowRoundess = 0.1f;

	if (IsKeyPressed(KEY_E)) {
		inventoryOpen = !inventoryOpen;
	}

	if (!inventoryOpen) {
		drawHotbar();
		return;
	}

	// inventory window background
	DrawRectangleRounded((Rectangle){windowBorder, windowBorder,
			(float)GetScreenWidth() - windowBorder * 2,
			(float)GetScreenHeight() - windowBorder * 2},
			windowRoundess, 0, WHITE);

	const float spacing = 10;
	int screenW = GetScreenWidth();

	// main inventory (40 slots, 10 cols) — left side
	float mainStartX = windowBorder + 40;
	float mainStartY = 100;

	auto mainRects = drawSlotGrid(inventory, 40, 10, mainStartX, mainStartY, spacing);
	handleGridDrag(inventory, 40, mainRects);

	// crafting grid — right side
	float craftStartX = screenW - windowBorder - 40 -
		(3 * (BLOCK_SIZE + spacing) + 25 + BLOCK_SIZE + 25);
	float craftStartY = 200;

	drawCraftingGrid(craftStartX, craftStartY, spacing);

	cancelDragIfReleased();
}

void Player::updateUI() { inventoryUpdate(); }
