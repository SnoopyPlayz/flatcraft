#include <cassert>
#include <cstdio>
#include <raylib.h>
#include "debug.hpp"
#include "vector.hpp"
#include <string>
#include <vector>
#include <map>
#include "map.hpp"
#include "rayUtils.hpp"


std::map<Vec3Int, Chunk> map;

void createChunk(Vec3Int pos){
	auto result = map.emplace(std::make_pair(pos, Chunk{}));
	assert(result.second);
}

Chunk& findChunk(Vec3Int pos){
	auto it = map.find(pos);
	assert(it != map.end() && "cant find chunk");

	return it->second;
}

Chunk& findOrCreateChunk(Vec3Int pos){
	if (validChunk(pos))
		return findChunk(pos);

	createChunk(pos);
	return findChunk(pos);
}

bool validChunk(Vec3Int pos){
	auto it = map.find(pos);
	if (it != map.end()) {
		return true;
	}
	return false;
}

Block getBlock(Vec3Int pos){
	Vec3Int chunkPos = pos / CHUNK_SIZE;
	Vec3Int localChunkPos = pos.mod(CHUNK_SIZE);

	if (!validChunk(chunkPos)) {
		return AIR;
	}

	Chunk& c = findChunk(chunkPos);
	return (Block)c.blocks[localChunkPos.x][localChunkPos.y][localChunkPos.z];
}

std::optional<Vec3Int> findTopBlock(int x, int y){
	for(int i = MAX_BLOCK_SEARCH_HEIGHT; i > -MAX_BLOCK_SEARCH_HEIGHT; i--){
		if (getBlock({x, i, y}) != AIR){
			return Vec3Int{x, i, y};
		}
	}
	return std::nullopt;
}

void setBlock(Vec3Int pos, int block){
	Vec3Int chunkPos = pos / CHUNK_SIZE;
	Vec3Int localChunkPos = pos.mod(CHUNK_SIZE);

	if (!validChunk(chunkPos)) {
		createChunk(chunkPos);
	}
	Chunk& c = findChunk(chunkPos);
	c.blocks[localChunkPos.x][localChunkPos.y][localChunkPos.z] = block;
	c.changed = true;
}

void drawMap(){
	for (const auto& pair : map) {
		for (int x{}; x < CHUNK_SIZE; x++) {
			for (int y{}; y < CHUNK_SIZE; y++) {
				for (int z{}; z < CHUNK_SIZE; z++) {
					if (pair.second.blocks[x][y][z] == AIR) {
						continue;
					}
					
					// assuming that there is a block at 34 0 0
					// 1 0 0 chunk position
					const Vec3Int chunkWorldPos = pair.first;
					// 2 0 0 block position in chunk
					const Vec3Int blockChunkPos = Vec3Int{x, y, z};

					// chunk world pos * size of chunk + block chunk pos * pixel size of block
					// 	     1 0 0 * 32 	   + 2 0 0   	     * 64 = 2176 0 0
					// 1 0 0 * 32 + 2 0 0 * 64 = 2176 0 0
					const Vec3Int worldPos = ((chunkWorldPos * CHUNK_SIZE) + blockChunkPos) * BLOCK_SIZE;

					if (pair.second.blocks[x][y][z] == GRASS) {
						drawTexture3D(useTexture("grass.png"), worldPos.toVec3(), ColorContrast(WHITE, y * -0.1));
					}

					if (pair.second.blocks[x][y][z] == STONE) {
						drawTexture3D(useTexture("stone.png"), worldPos.toVec3(), ColorContrast(WHITE, y * -0.1));
					}
				}
			}
		}
	}
}
