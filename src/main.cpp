#include "camera.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "raylib.h"

int main(){
	initNetwork();
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

		updateNetwork();

		drawAllTextures3D();


		DrawFPS(10, 10);
		EndMode2D();
		debug.draw();
		EndDrawing();
	}
	enet_deinitialize();
	CloseWindow();
	return 0;
}
