#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include "camera.hpp"
#include "rayUtils.hpp"
#include "map.hpp"
#include "debug.hpp"
#include "player.hpp"

Player player;

void Player::update(){
	// Select block
	for (int key = KEY_ZERO; key <= KEY_NINE; key++) {
		if (IsKeyPressed(key)) {
			selectedBlock = static_cast<Block>(key - KEY_ZERO);
		}
	}

	const float playerSpeed = 10;
	// Draw player
	DrawTexture(useTexture("player.png"), pos.x, pos.z, WHITE);

	Vector3 vel = {0, 0, 0};

	vel.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
	vel.y = IsKeyDown(KEY_SPACE) - IsKeyDown(KEY_LEFT_CONTROL);
	vel.z = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);

	pos += Vector3Normalize(vel) * playerSpeed;

	Vector2 mouseScreen = GetMousePosition();
	Vector2 m = GetScreenToWorld2D(mouseScreen, playerCamera.camera);
	int x = (int)std::floor(m.x / (float)BLOCK_SIZE);
	int z = (int)std::floor(m.y / (float)BLOCK_SIZE);

	auto topBlock = findTopBlock(x, z);

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		if (topBlock.has_value()) {
			setBlock({x, topBlock->y + 1, z}, selectedBlock);
		} else {
			setBlock({x, 0, z}, selectedBlock);
		}
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		if (topBlock.has_value()) {
			setBlock({x, topBlock->y, z}, AIR);
		} else {
			setBlock({x, 0, z}, AIR);
		}
	}

	//Debug info
	debug.addMessage("Player pos: %R x: " + std::to_string(pos.x) + " %G Y: " + std::to_string(pos.y) + " %B Z: " + std::to_string(pos.z));
	debug.addMessage("cursor pos: %R x: " + std::to_string(x) + " %G Y: " 
			+ (topBlock.has_value() ? std::to_string(topBlock->y) : "N/A")
			+ " %B Z: " + std::to_string(z));
}
