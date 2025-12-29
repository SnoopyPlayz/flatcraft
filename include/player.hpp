#pragma once

#include "map.hpp"
#include "raylib.h"

class Player{
	public:
		Player();
		void update();
		Vector3 pos;
		Block selctedBlock = GRASS;
};

extern Player player;
