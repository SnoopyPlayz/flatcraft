#include "camera.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "raylib.h"

int main(){
	testNetwork();
	InitWindow(1280, 720, "flatCraft");
	SetTargetFPS(60);
	setAndLoadFont("Roboto-VariableFont_wdth,wght.ttf");

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(playerCamera.camera);

		drawMap();
		player.update();
		playerCamera.update();
		drawAllTextures3D();

		EndMode2D();
		debug.draw();
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
