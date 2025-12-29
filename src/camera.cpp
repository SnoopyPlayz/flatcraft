#include "player.hpp"
#include <raylib.h>
#include <camera.hpp>
#include "map.hpp"

PlayerCamera playerCamera;

PlayerCamera::PlayerCamera(){
	camera.target = {0,0};
	camera.offset = {0,0};
	camera.zoom = 1;
	camera.rotation = 0;
}

void PlayerCamera::update(){
	const int cameraSpeed = 10;
	camera.target.x = player.pos.x;
	camera.target.y = player.pos.z;
	camera.offset.x = GetScreenWidth() * 0.5 - BLOCK_SIZE * 0.5;
	camera.offset.y = GetScreenHeight() * 0.5 - BLOCK_SIZE * 0.5;
}
