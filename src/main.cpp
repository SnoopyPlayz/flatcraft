#include "camera.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "raylib.h"
#include "server.hpp"
#include "client.hpp"
#include <enet/enet.h>
#include <unistd.h>

int main(){
	testNetwork();
	InitWindow(1280, 720, "flatCraft");
	SetTargetFPS(60);
	setAndLoadFont("Roboto-VariableFont_wdth,wght.ttf");
	bool server = false;
	bool initalizedNetworking = false;

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(playerCamera.camera);

		drawMap();
		player.update();
		playerCamera.update();

		if (IsKeyDown(KEY_I)) {
			sleep(5);
		}

		if (IsKeyPressed(KEY_L)) {
			server = true;
			initalizedNetworking = hostServer();
		}

		if (IsKeyPressed(KEY_K)) {
			server = false;
			initalizedNetworking = createClient();
		}
		if (initalizedNetworking) {
			if (server) {
				updateServer();
			}
			else {
				updateClient();
			}
		}

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
