#include <camera.hpp>
#include "map.hpp"
#include "player.hpp"

int main(){
	InitWindow(1280, 720, "flatCraft");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(playerCamera.camera);

		drawMap();
		player.update();
		playerCamera.update();

		EndMode2D();
		EndDrawing();
	}

	return 0;
}

