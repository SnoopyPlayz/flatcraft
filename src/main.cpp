#include "camera.hpp"
#include "item.hpp"
#include "map.hpp"
#include "player.hpp"
#include "network.hpp"
#include "debug.hpp"
#include "rayUtils.hpp"
#include "client.hpp"
#include "worldGen.hpp"
#include "gameConfig.hpp"
#include <chrono>
#include <cstdint>
#include <raylib.h>
#include <string>
#include <thread>

struct WindowArgs {
	bool useInternalServer = true;
	std::string networkHost = "localhost";
	uint16_t networkPort = 1236;
	bool hasWindowPos = false;
	int windowPosX = 0;
	int windowPosY = 0;
};

static WindowArgs parseWindowArgs(int argc, char *argv[]) {
	WindowArgs args;
	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg.starts_with("windowPos") && i + 2 < argc) {
			args.windowPosX = std::stoi(argv[++i]);
			args.windowPosY = std::stoi(argv[++i]);
			args.hasWindowPos = true;
			continue;
		}
		if (arg == "--join" && i + 1 < argc) {
			args.useInternalServer = false;
			args.networkHost = argv[++i];
			if (i + 1 < argc) {
				std::string maybePort(argv[i + 1]);
				if (!maybePort.starts_with("--") && !maybePort.starts_with("windowPos")) {
					args.networkPort = static_cast<uint16_t>(std::stoi(maybePort));
					++i;
				}
			}
		}
	}
	return args;
}

int main(int argc, char *argv[]) {
	WindowArgs args = parseWindowArgs(argc, argv);

	configureNetwork(args.useInternalServer, args.networkHost, args.networkPort);
	initNetwork();
	//SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 720, "flatCraft");
	if (args.hasWindowPos) SetWindowPosition(1920 + args.windowPosX, args.windowPosY);
	setAndLoadFont("Roboto-VariableFont_wdth,wght.ttf");
	map.createShadowTexture();

	map.worldGenInit();

	using Clock = std::chrono::steady_clock;
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

		if (accumulator < FIXED_FRAME_TIME) {
			continue;
		}
		accumulator -= FIXED_FRAME_TIME;
		framesThisWindow++;

		if (fpsWindowElapsed >= fpsUpdateWindow) {
			displayedFps = static_cast<int>(framesThisWindow / fpsWindowElapsed.count());
			fpsWindowElapsed = std::chrono::duration<double>::zero();
			framesThisWindow = 0;
		}

		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(playerCamera.camera);

		map.drawMap();
		player.update();
		playerCamera.update();
		map.createShadowsForMap();
		drawItems();

		preparePacket();
		processRecevedPacket();

		updateNetwork();

		drawAllTextures3D();

		map.debugMap();
		EndMode2D();
		player.updateUI();
		debug.addMessage("fps : %G " + std::to_string(displayedFps));
		debug.draw();
		EndDrawing();
	}
	shutdownNetwork();
	enet_deinitialize();
	unloadShaders();
	CloseWindow();
	return 0;
}
