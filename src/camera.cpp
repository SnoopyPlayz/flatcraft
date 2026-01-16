#include "camera.hpp"
#include "player.hpp"
#include <raylib.h>

PlayerCamera playerCamera;
const float EPSILON = 0.00001f;

PlayerCamera::PlayerCamera() {
	camera.target = {0, -1, EPSILON};
	camera.position = {0, 1, 0};
	camera.fovy = 10; //zoom
	camera.projection = CAMERA_ORTHOGRAPHIC;
	camera.up = {0, 1, 0};
}

void PlayerCamera::update() {
	camera.position.z = player.pos.z;
	camera.target.z = player.pos.z - EPSILON;

	camera.position.x = player.pos.x;
	camera.target.x = player.pos.x;
}
