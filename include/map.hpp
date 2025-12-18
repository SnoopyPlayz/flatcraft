#pragma once
#include <cstdint>
#include <vector.hpp>
#include <map>

const int CHUNK_SIZE = 32;

struct Chunk {
	uint8_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

typedef enum Block{
	AIR,
	GRASS,
} Block;

extern std::map<Vec3Int, Chunk> map;

void createChunk(Vec3Int xyz);
Chunk& findChunk(Vec3Int search_key);

bool validChunk(Vec3Int pos);

Block getBlock(Vec3Int pos);
void setBlock(Vec3Int pos, int block);

Vec3Int findTopBlock(int x, int y);

