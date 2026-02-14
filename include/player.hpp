#pragma once

#include "map.hpp"
#include "raylib.h"
#include "network.hpp"
#include <mutex>
#include <vector>

//std::mutex blockUpdateMutex;
extern std::vector<BlockUpdatePacket> blockUpdates;
class Player{
	public:
		void update();
		Vector3 pos = {0, 0.5, 0};
		Block selectedBlock = GRASS;
		Vector3 velocity = {0, 0, 0};
};

struct PlayerData{
	Player player;
	enet_uint32 peer;
};

extern Player player;
