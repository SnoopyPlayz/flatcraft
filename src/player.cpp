#include <cmath>
#include <raylib.h>
#include "camera.hpp"
#include "rayUtils.hpp"
#include "map.hpp"
#include "player.hpp"

Player player;

void Player::update(){

	for (int key = KEY_ZERO; key <= KEY_NINE; key++) {
		if (IsKeyPressed(key)) {
			selectedBlock = static_cast<Block>(key - KEY_ZERO);
		}
	}

	DrawTexture(useTexture("player.png"), pos.x, pos.z, WHITE);
	const float playerSpeed = 10;

	if (IsKeyDown(KEY_S))
		pos.z += playerSpeed;

	if (IsKeyDown(KEY_A))
		pos.x -= playerSpeed;

	if (IsKeyDown(KEY_D))
		pos.x += playerSpeed;

	if (IsKeyDown(KEY_W))
		pos.z -= playerSpeed;

	if (IsKeyPressed(KEY_SPACE))
		pos.y += 1;

	if (IsKeyPressed(KEY_LEFT_CONTROL))
		pos.y -= 1;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Vector2 mouseScreen = GetMousePosition();
		Vector2 m = GetScreenToWorld2D(mouseScreen, playerCamera.camera);
		int x = (int)std::floor(m.x / (float)BLOCK_SIZE);
		int z = (int)std::floor(m.y / (float)BLOCK_SIZE);

		auto topBlock = findTopBlock(x, z);
		if (topBlock.has_value()) {
			setBlock({x, topBlock->y + 1, z}, selectedBlock);
		} else {
			setBlock({x, 0, z}, selectedBlock);
		}
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		Vector2 mouseScreen = GetMousePosition();
		Vector2 m = GetScreenToWorld2D(mouseScreen, playerCamera.camera);
		int x = (int)std::floor(m.x / (float)BLOCK_SIZE);
		int z = (int)std::floor(m.y / (float)BLOCK_SIZE);

		auto topBlock = findTopBlock(x, z);
		if (topBlock.has_value()) {
			setBlock({x, topBlock->y, z}, AIR);
		} else {
			setBlock({x, 0, z}, AIR);
		}
	}


}
