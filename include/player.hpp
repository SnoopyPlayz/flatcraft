#pragma once

#include "map.hpp"
#include "raylib.h"

class Player{
	public:
		void update();
		Vector3 pos = {0, 0.5, 0};
		Block selectedBlock = GRASS;
};

extern Player player;
