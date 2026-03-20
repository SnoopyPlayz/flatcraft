#include <cmath>
#include <cstdlib>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include "camera.hpp"
#include "rayUtils.hpp"
#include "map.hpp"
#include "debug.hpp"
#include "player.hpp"
#include "network.hpp"
#include "vector.hpp"
#include "physics.hpp"
#include <iostream>

Player player;
std::vector<BlockUpdatePacket> blockUpdates;

bool inventoryOpen = false;

int in = 0;
void Player::inventoryUpdate(){
	const float windowBorder = 50;
	const float windowRoundess = 0.1;
	const int rowSize = 10; // amount of elements in a row

	const int rowStart = 100;

	if (IsKeyPressed(KEY_E)) {
		inventory[in++] = (Block)(STONE);
		inventory[in++] = (Block)(GRASS);
		inventory[in++] = (Block)(CRAFTING_TABLE);
		std::cout << "inventory update " + std::to_string(in) << std::endl;
		inventoryOpen = !inventoryOpen;
	}
	if (!inventoryOpen) {
		return;
	}

	DrawRectangleRounded((Rectangle){windowBorder, windowBorder, (float)GetScreenWidth() - windowBorder * 2, (float)GetScreenHeight() - windowBorder * 2}, windowRoundess, 0, WHITE);

	int y = 100;
	int x = 1;
	// drawGrid and draw SelectedBlock
	static bool selected = false;
	static Block selectedBlock = AIR;
	static int lastPos;

	for (int i{}; i < PLAYER_INVENTORY_SIZE; i++) {
		float pixelPosX = 1.2 * BLOCK_SIZE * x;

		if (x > rowSize || pixelPosX + BLOCK_SIZE > GetScreenWidth()){
			x = 1;
			y += 100;
		}

		pixelPosX = 1.2 * BLOCK_SIZE * x;

		// item background
		const float itemGridOffset = 5;
		DrawRectangleRounded((Rectangle){pixelPosX - itemGridOffset, (float)y - itemGridOffset, BLOCK_SIZE + itemGridOffset * 2, BLOCK_SIZE + itemGridOffset * 2}, windowRoundess,  1, DARKGRAY);

		x++;

		// draw item Tex
		if (inventory[i] != AIR){
			DrawTexture(useTexture(getEnumName((Block)inventory[i]) + ".png"), pixelPosX, y, WHITE);
		}

		// colision with cursor
		if (AABBcolBox(pixelPosX,y,BLOCK_SIZE,GetMouseX(),GetMouseY(), 1)){
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inventory[i] != AIR){
				selected = true;
				selectedBlock = (Block)inventory[i];
				inventory[i] = AIR;
				lastPos = i;
			}

			if(selected){
				if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
					inventory[lastPos] = (Block)inventory[i];
					inventory[i] = selectedBlock;
					selected = false;
					selectedBlock = AIR;
				}
			}

			DrawRectangle(pixelPosX, y, BLOCK_SIZE, BLOCK_SIZE, ColorAlpha(GRAY, 0.5));
		}

		// draw selectedBlock
		if(selected){
			DrawTexture(useTexture(getEnumName(selectedBlock) + ".png"), GetMouseX() - BLOCK_SIZE * 0.5, GetMouseY() - BLOCK_SIZE * 0.5, WHITE);
		}
	}
}

void Player::updateUI(){
	inventoryUpdate();
}

void Player::updateMovement(){
	const float playerRunningSpeed = 0.18;
	const float playerSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? playerRunningSpeed : 0.12;
	const Vector3 playerSize = {1.0f, 1.0f, 1.0f};
	const Vector3 blockSize = {1.0f, 1.0f, 1.0f};
	const int collisionRange = 2;

	Vector3 movementInput = {0.0f, 0.0f, 0.0f};
	movementInput.x = static_cast<float>(IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
	movementInput.z = static_cast<float>(IsKeyDown(KEY_S) - IsKeyDown(KEY_W));

	if (Vector3LengthSqr(movementInput) > 0.0f) {
		movementInput = Vector3Normalize(movementInput);
	}

	velocity.x = Lerp(velocity.x, movementInput.x * playerSpeed, PLAYER_ACCELERATION_SPEED);
	velocity.z = Lerp(velocity.z, movementInput.z * playerSpeed, PLAYER_ACCELERATION_SPEED);

	if (grounded && IsKeyPressed(KEY_SPACE)) {
		velocity.y = JUMP_VELOCITY;
		grounded = false;
	} else {
		velocity.y = applyGravity(velocity.y);
	}

	auto resolveAxisCollisions = [&](int axis) {
		Vector3 playerCenter = pos + (playerSize * 0.5f);
		const Vec3Int centerBlock = {
			static_cast<int32_t>(std::floor(playerCenter.x)),
			static_cast<int32_t>(std::floor(playerCenter.y)),
			static_cast<int32_t>(std::floor(playerCenter.z))
		};

		RANGE(collisionRange) {
			const Vec3Int blockPos = centerBlock + Vec3Int{x, y, z};
			if (getBlock(blockPos) == AIR) {
				continue;
			}

			const Vector3 blockCenter = blockPos.toVec3() + (blockSize * 0.5f);
			const float axisResponse = AABBcollisionResponseAxis(playerCenter, playerSize, blockCenter, blockSize, axis);
			if (axisResponse == 0.0f) {
				continue;
			}

			if (axis == 0) {
				pos.x -= axisResponse;
				playerCenter.x -= axisResponse;
				velocity.x = 0.0f;
			} else if (axis == 1) {
				pos.y += abs(axisResponse);
				playerCenter.y += abs(axisResponse);
				if (axisResponse < 0.0f && velocity.y <= 0.0f) {
					grounded = true;
				}
				velocity.y = 0.0f;
			} else {
				pos.z -= axisResponse;
				playerCenter.z -= axisResponse;
				velocity.z = 0.0f;
			}
		}
	};

	grounded = false;
	pos.y += velocity.y;
	resolveAxisCollisions(1);

	pos.x += velocity.x;
	resolveAxisCollisions(0);

	pos.z += velocity.z;
	resolveAxisCollisions(2);
}

void Player::updateBlockPlacingBreaking(){
	const Vector3 playerSize = {1.0f, 1.0f, 1.0f};
	const Vector3 blockSize = {1.0f, 1.0f, 1.0f};
	auto canPlaceAt = [&](const Vec3Int& blockPos) {
		const Vector3 playerCenter = pos + (playerSize * 0.5f);
		const Vector3 blockCenter = blockPos.toVec3() + (blockSize * 0.5f);
		return !AABBcolBox(playerCenter - (playerSize * 0.5f), playerSize,
			blockCenter - (blockSize * 0.5f), blockSize);
	};

	// Select block
	for (int key = KEY_ZERO; key <= KEY_NINE; key++) {
		if (IsKeyPressed(key)) {
			selectedBlock = static_cast<Block>(key - KEY_ZERO);
		}
	}

	Vector2 mouseScreen = GetMousePosition();
	Vector2 m = GetScreenToWorld2D(mouseScreen, playerCamera.camera);
	int x = std::floor(m.x / (float)BLOCK_SIZE);
	int z = std::floor(m.y / (float)BLOCK_SIZE);

	auto topBlock = findTopBlock(x, z);

	if (topBlock.has_value()) {
		drawRect3D({(float)x * BLOCK_SIZE ,(float)(topBlock->y + 1) * BLOCK_SIZE, (float)z * BLOCK_SIZE}, ColorAlpha(DARKGRAY, 0.4f));
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		//std::lock_guard<std::mutex> lock(blockUpdateMutex);
		if (topBlock.has_value()) {
			const Vec3Int targetPos = {x, topBlock->y + 1, z};
			if (canPlaceAt(targetPos)) {
				setBlock(targetPos, selectedBlock);
				blockUpdates.push_back({targetPos, selectedBlock});
			}

		} else {
			const Vec3Int targetPos = {x, 0, z};
			if (canPlaceAt(targetPos)) {
				setBlock(targetPos, selectedBlock);
				blockUpdates.push_back({targetPos, selectedBlock});
			}
		}
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		//std::lock_guard<std::mutex> lock(blockUpdateMutex);
		if (topBlock.has_value()) {
			setBlock({x, topBlock->y, z}, AIR);
			blockUpdates.push_back({{x, topBlock->y, z}, AIR});
		} else {
			setBlock({x, 0, z}, AIR);
			blockUpdates.push_back({{x, 0, z}, AIR});
		}
	}

	debug.addMessage("cursor pos: %R x: " + std::to_string(x) + " %G Y: " 
			+ (topBlock.has_value() ? std::to_string(topBlock->y) : "N/A")
			+ " %B Z: " + std::to_string(z));
}

const float PLAYER_ACCELERATION_SPEED = 0.3;
void Player::update(){

	//zoom
	static float scroll;
	scroll += GetMouseWheelMove() * 0.1f;
	if (IsKeyDown(KEY_C)){
		if (scroll < 0.1f) {
			scroll = 0.1f;
		}
		playerCamera.camera.zoom = scroll;
	} else {
		playerCamera.camera.zoom = 1.0f;
		scroll = 1;
	}

	updateMovement();
	updateBlockPlacingBreaking();
	// Draw player
	drawTexture3D(useTexture("player.png"), pos * BLOCK_SIZE, WHITE);

	debug.addMessage("Player pos: %R x: " + std::to_string(pos.x) + " %G Y: " + std::to_string(pos.y) + " %B Z: " + std::to_string(pos.z));
}
