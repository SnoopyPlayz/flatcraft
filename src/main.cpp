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
#include <thread>
#include <unistd.h>

int main(){
	testNetwork();
	InitWindow(1280, 720, "flatCraft");
	SetTargetFPS(60);
	setAndLoadFont("Roboto-VariableFont_wdth,wght.ttf");
	std::jthread networkThread;

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(playerCamera.camera);

		drawMap();
		player.update();
		playerCamera.update();

		if (IsKeyDown(KEY_I)) {
			sleep(1);
		}

		if (IsKeyPressed(KEY_L)) {
			if (hostServer()){
				networkThread = std::jthread(updateServer);
			}
		}

		if (IsKeyPressed(KEY_K)) {
			if (createClient()) {
				networkThread = std::jthread(updateClient);
			}
		}

		drawClients();
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
