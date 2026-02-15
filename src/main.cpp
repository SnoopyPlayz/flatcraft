#include "camera.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "raylib.h"
#include <cstdint>
#include <string>

int main(int argc, char *argv[]) {
	bool useInternalServer = true;
	std::string networkHost = "localhost";
	uint16_t networkPort = 1236;
	bool hasWindowPos = false;
	int windowPosX = 0;
	int windowPosY = 0;

	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg.starts_with("windowPos") && i + 2 < argc) {
			windowPosX = std::stoi(argv[++i]);
			windowPosY = std::stoi(argv[++i]);
			hasWindowPos = true;
			continue;
		}
		if (arg == "--join" && i + 1 < argc) {
			useInternalServer = false;
			networkHost = argv[++i];
			if (i + 1 < argc) {
				std::string maybePort(argv[i + 1]);
				if (!maybePort.starts_with("--") && !maybePort.starts_with("windowPos")) {
					networkPort = static_cast<uint16_t>(std::stoi(maybePort));
					++i;
				}
			}
		}
	}

	configureNetwork(useInternalServer, networkHost, networkPort);
	initNetwork();
	InitWindow(1280, 720, "flatCraft");
	if (hasWindowPos) SetWindowPosition(1920 + windowPosX, windowPosY);
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
	shutdownNetwork();
	enet_deinitialize();
	CloseWindow();
	return 0;
}
