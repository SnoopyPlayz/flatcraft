#include "camera.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "raylib.h"

int main(int argc, char *argv[]) {
	initNetwork();
	InitWindow(1280, 720, "flatCraft");
	SetTargetFPS(60);
	setAndLoadFont("Roboto-VariableFont_wdth,wght.ttf");

	if (argc > 3) {
		std::string arg1(argv[1]);
		if (arg1.starts_with("windowPos")) {
			std::string widthArg(argv[2]);
			std::string heightArg(argv[3]);
			int x = std::stoi(widthArg);
			int y = std::stoi(heightArg);
			SetWindowPosition(1920 + x, y);
		} 
	}

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
