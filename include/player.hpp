#pragma once

#include "map.hpp"
#include "raylib.h"

class Player{
	public:
		void update();
		Vector3 pos;
		Block selectedBlock = GRASS;
};

extern Player player;
