#include <cmath>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include "camera.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "map.hpp"
#include "player.hpp"

Player player;

void Player::update(){
	const float playerSpeed = 0.1;

	// Select block
	for (int key = KEY_ZERO; key <= KEY_NINE; key++) {
		if (IsKeyPressed(key)) {
			selectedBlock = static_cast<Block>(key - KEY_ZERO);
		}
	}

	// Draw player
	DrawTexture3D(pos, "player.png", WHITE);

	Vector3 vel = {0, 0, 0};

	vel.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
	vel.y = IsKeyDown(KEY_SPACE) - IsKeyDown(KEY_LEFT_CONTROL);
	vel.z = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);

	pos += Vector3Normalize(vel) * playerSpeed;

	Vector2 mouseScreen = GetMousePosition();
	Vector3 m = GetScreenToWorldRay(mouseScreen, playerCamera.camera).position;

	int x = std::round(m.x);
	int z = std::round(m.z);

	auto topBlock = findTopBlock(x, z);

	// place/remove block
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		if (topBlock.has_value()) {
			setBlock({x, topBlock->y, z}, AIR);
		} else {
			setBlock({x, 0, z}, AIR);
		}
	}


	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		if (topBlock.has_value()) {
			setBlock({x, topBlock->y + 1, z}, selectedBlock);
		} else {
			setBlock({x, 0, z}, selectedBlock);
		}
	}

	//Debug info
	debug.addMessage("Player pos: %RX: " + std::to_string(pos.x) + " %G Y: " + std::to_string(pos.y) + " %B Z: " + std::to_string(pos.z));
	debug.addMessage("cursor pos: %RX: " + std::to_string(x) + " %BZ: " + std::to_string(z));
}
