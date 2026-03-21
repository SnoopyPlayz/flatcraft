#pragma once

#include "map.hpp"
#include "raylib.h"
#include "network.hpp"
#include <sys/types.h>
#include <vector>

//std::mutex blockUpdateMutex;
extern std::vector<BlockUpdatePacket> blockUpdates;
extern const float PLAYER_ACCELERATION_SPEED;

const int PLAYER_INVENTORY_SIZE = 40;
class Player{
	public:
		void updateUI();
		void update();
		Vector3 pos = {0, 20, 0};
		Block selectedBlock = GRASS;
		Vector3 velocity = {0, 0, 0};
		uint8_t inventory[PLAYER_INVENTORY_SIZE];
		int health = 20;
	private:
		void inventoryUpdate();
		void updateMovement();
		void updateBlockPlacingBreaking();
};

struct PlayerData{
	Player player;
	enet_uint32 peer;
};

extern Player player;
