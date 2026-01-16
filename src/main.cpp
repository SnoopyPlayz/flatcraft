#include "camera.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "rayUtils.hpp"
#include "raylib.h"
#include "debug.hpp"
#include <iostream>

int main(){
	testNetwork();
	InitWindow(1280, 720, "flatCraft");
	SetTargetFPS(60);

	initRayUtils();

	
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(DARKGRAY);
		BeginMode3D(playerCamera.camera);

		player.update();
		playerCamera.update();
		drawMap();

		DrawGrid(100, 1);

		EndMode3D();

		debug.draw();
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
