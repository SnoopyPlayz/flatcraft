#pragma once
#include "map.hpp"
#include "player.hpp"
#include "enet/enet.h"

int testNetwork();

struct ChunkData{
	Chunk chunk;
	Vec3Int pos;
};

struct PlayerData{
	Player player;
	enet_uint32 peer;
};
