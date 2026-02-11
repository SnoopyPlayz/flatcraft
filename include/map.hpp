#pragma once
#include <cstdint>
#include "vector.hpp"
#include <map>
#include <optional>

const int CHUNK_SIZE = 16;
const int BLOCK_SIZE = 64;
const int MAX_BLOCK_SEARCH_HEIGHT = 100;

struct Chunk {
	bool changed = false; // has chunk changed this frame
	uint8_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

typedef enum Block{
	AIR,
	GRASS,
	STONE,
} Block;

extern std::map<Vec3Int, Chunk> map;


void drawMap();

Chunk& findOrCreateChunk(Vec3Int pos);

void createChunk(Vec3Int xyz);
Chunk& findChunk(Vec3Int search_key);

bool validChunk(Vec3Int pos);

Block getBlock(Vec3Int pos);
void setBlock(Vec3Int pos, int block);

std::optional<Vec3Int> findTopBlock(int x, int y);

