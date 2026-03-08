#include "camera.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "worldGen.hpp"
#include <chrono>
#include <cstdint>
#include <raylib.h>
#include <string>
#include <thread>

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
	setAndLoadFont("Roboto-VariableFont_wdth,wght.ttf");
	createShadowTexture();

	worldGenInit();

	using Clock = std::chrono::steady_clock;
	constexpr auto fixedFrameTime = std::chrono::duration<double>(1.0 / 60.0);
	constexpr auto maxFrameTime = std::chrono::duration<double>(0.1);
	constexpr auto fpsUpdateWindow = std::chrono::seconds(1);

	auto previousFrameTime = Clock::now();
	std::chrono::duration<double> accumulator{0.0};
	std::chrono::duration<double> fpsWindowElapsed{0.0};
	int framesThisWindow = 0;
	int displayedFps = 0;

	while (!WindowShouldClose()) {
		const auto currentFrameTime = Clock::now();
		const auto deltaTime = std::chrono::duration<double>(currentFrameTime - previousFrameTime);
		previousFrameTime = currentFrameTime;
		fpsWindowElapsed += deltaTime;

		accumulator += deltaTime;
		if (accumulator > maxFrameTime) {
			accumulator = maxFrameTime;
			debug.addMessage("%R lagging " + std::to_string(accumulator.count()));
		}

		if (accumulator < fixedFrameTime) {
			continue;
		}
		accumulator -= fixedFrameTime;
		framesThisWindow++;

		if (fpsWindowElapsed >= fpsUpdateWindow) {
			displayedFps = static_cast<int>(framesThisWindow / fpsWindowElapsed.count());
			fpsWindowElapsed = std::chrono::duration<double>::zero();
			framesThisWindow = 0;
		}

		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(playerCamera.camera);

		drawMap();
		player.update();
		playerCamera.update();
		createShadowsForMap();

		updateNetwork();

		drawAllTextures3D();

		debugMap();
		EndMode2D();
		debug.addMessage("dirty : %G " + std::to_string(displayedFps));
		debug.draw();
		EndDrawing();
	}
	shutdownNetwork();
	enet_deinitialize();
	unloadShaders();
	CloseWindow();
	return 0;
}
