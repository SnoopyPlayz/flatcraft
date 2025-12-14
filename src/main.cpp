#include <cassert>
#include <map>
#include <stdio.h>
#include <raylib.h>
#include <utility>
#include <vector>
#include "rayUtils.hpp"
#include <cstdint>
#include <cmath>
#include <iostream>
#include <map.hpp>


int main(){
	InitWindow(1280, 720, "flatCraft");
	SetTargetFPS(60);

	setBlock({0,0,0},GRASS);

	const int blockSize = 64;
	Camera2D camera = { 0 };
	camera.target = {0,0};
	camera.offset = {0,0};
	camera.zoom = 1;
	camera.rotation = 0;

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(camera);

		const int cameraSpeed = 10;

		if (IsKeyDown(KEY_S))
			camera.target.y += cameraSpeed;

		if (IsKeyDown(KEY_A))
			camera.target.x -= cameraSpeed;

		if (IsKeyDown(KEY_D))
			camera.target.x += cameraSpeed;

		if (IsKeyDown(KEY_W))
			camera.target.y -= cameraSpeed;


		for (const auto& pair : map) {
			for (int x{}; x < CHUNK_SIZE; x++) {
				for (int y{}; y < CHUNK_SIZE; y++) {
					for (int z{}; z < CHUNK_SIZE; z++) {
						if (pair.second.blocks[x][y][z] == GRASS) {
							const int chunk_world_x = pair.first.x * CHUNK_SIZE * blockSize;
							const int chunk_world_z = pair.first.z * CHUNK_SIZE * blockSize;

							// Local Block Position (relative to chunk start)
							const int block_local_x = x * blockSize;
							const int block_local_z = z * blockSize;

							// Final World Position
							const int world_x = chunk_world_x + block_local_x;
							const int world_z = chunk_world_z + block_local_z;
							DrawTexture(useTexture("grass.png"), world_x, world_z, WHITE);
						}
					}
				}
			}
		}


		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			Vector2 mouseScreen = GetMousePosition();
			Vector2 m = GetScreenToWorld2D(mouseScreen, camera);

			setBlock({(int)std::floor(m.x / (float)blockSize),0,(int)std::floor(m.y / (float)blockSize)}, 1);
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			Vector2 mouseScreen = GetMousePosition();
			Vector2 m = GetScreenToWorld2D(mouseScreen, camera);

			setBlock({(int)std::floor(m.x / (float)blockSize),0,(int)std::floor(m.y / (float)blockSize)}, 0);
		}

		EndMode2D();
		EndDrawing();
	}

	return 0;
}
