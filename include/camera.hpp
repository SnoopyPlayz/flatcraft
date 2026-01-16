#pragma once
#include <raylib.h>

class PlayerCamera{
	public:
		PlayerCamera();
		void update();
		Camera3D camera;
};

extern PlayerCamera playerCamera;
