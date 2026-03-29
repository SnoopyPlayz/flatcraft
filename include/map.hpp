#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include "vector.hpp"

constexpr int CHUNK_SIZE = 16;
constexpr int BLOCK_SIZE = 64;
constexpr int MAX_BLOCK_SEARCH_HEIGHT = 100;

struct Chunk {
	bool changed = false; // has chunk changed this frame
	bool generated = false;
	uint8_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]{};
};

enum Block {
	AIR,
	GRASS,
	STONE,
	CRAFTING_TABLE,
};

class Map {
public:
	void createChunk(Vec3Int pos);
	Chunk& findChunk(Vec3Int searchKey);
	Chunk& findOrCreateChunk(Vec3Int pos);
	bool validChunk(Vec3Int pos);
	Block getBlock(Vec3Int pos);
	void setBlock(Vec3Int pos, Block block);
	void worldGenInit();
	void genChunk(Vec3Int chunkPos);
	void markShadowAffectedChunksChanged(Vec3Int chunkPos);
	std::optional<Vec3Int> findTopBlock(int x, int y);
	void createShadowTexture();
	void createShadowsForMap();
	void debugMap();
	void drawMap();

private:
	std::map<Vec3Int, Chunk> chunks;
};

extern Map map;

#define FOR_XYZ(x1,y1,z1) \
	for (int x{}; x < x1; ++x)  \
		for (int y{}; y < y1; ++y) \
			for (int z{}; z < z1; ++z)

#define RADUIS(radius) \
	for (int x = -radius; x <= radius; ++x) \
		for (int y = -radius; y <= radius; ++y) \
			for (int z = -radius; z <= radius; ++z) \
