#include <cassert>
#include <cstdio>
#include <raylib.h>
#include <vector.hpp>
#include <map>
#include "map.hpp"
#include "rayUtils.hpp"

std::map<Vec3Int, Chunk> map;
void createChunk(Vec3Int xyz){
	printf("Created Chunk %d %d %d", xyz.x, xyz.y, xyz.z);
	auto result = map.emplace(std::make_pair(xyz, Chunk{}));
	assert(result.second);
}

Chunk& findChunk(Vec3Int search_key){
	auto it = map.find(search_key);
	assert(it != map.end());

	return it->second;
}

/*std::vector<Chunk> getAllChunks(){
	std::vector<Chunk> result;
	for (const auto& pair : map) {
		result.emplace(pair.second);
	}
}*/

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

Vec3Int findTopBlock(int x, int y){
	for(int i = 100; i > -100; i--){
		if (getBlock({x, i, y}) != AIR){
			return {x, i, y};
		}
	}
	return {-999,-999,-999}; // TODO change this
}


void setBlock(Vec3Int pos, int block){
	Vec3Int chunkPos = pos / CHUNK_SIZE;
	Vec3Int localChunkPos = pos.mod(CHUNK_SIZE);

	if (!validChunk(chunkPos)) {
		createChunk(chunkPos);
	}
	Chunk& c = findChunk(chunkPos);
	c.blocks[localChunkPos.x][localChunkPos.y][localChunkPos.z] = block;
	printf("placing block on X: %d \n", math::mod(pos.x,CHUNK_SIZE));
	printf("placing block on X: %d \n", math::mod(pos.y,CHUNK_SIZE));
	printf("placing block on z: %d \n", math::mod(pos.z,CHUNK_SIZE));
}

void drawMap(){
	for (const auto& pair : map) {
		for (int x{}; x < CHUNK_SIZE; x++) {
			for (int y{}; y < CHUNK_SIZE; y++) {
				for (int z{}; z < CHUNK_SIZE; z++) {

					const int chunk_world_x = pair.first.x * CHUNK_SIZE * BLOCK_SIZE;
					const int chunk_world_z = pair.first.z * CHUNK_SIZE * BLOCK_SIZE;

					// Local Block Position (relative to chunk start)
					const int block_local_x = x * BLOCK_SIZE;
					const int block_local_z = z * BLOCK_SIZE;

					// Final World Position
					const int world_x = chunk_world_x + block_local_x;
					const int world_z = chunk_world_z + block_local_z;

					if (pair.second.blocks[x][y][z] == GRASS) {
						DrawTexture(useTexture("grass.png"), world_x, world_z, ColorContrast(WHITE, y * -0.1));
					}

					if (pair.second.blocks[x][y][z] == STONE) {
						DrawTexture(useTexture("stone.png"), world_x, world_z, ColorContrast(WHITE, y * -0.1));
					}
				}
			}
		}
	}
}
