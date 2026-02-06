#pragma once
#include <raylib.h>

class PlayerCamera{
	public:
		PlayerCamera();
		void update();
		Camera2D camera;
};

extern PlayerCamera playerCamera;
