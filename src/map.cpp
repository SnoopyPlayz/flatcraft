#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <raylib.h>
#include "debug.hpp"
#include "vector.hpp"
#include <string>
#include <vector>
#include <map>
#include "map.hpp"
#include "rayUtils.hpp"
#include "player.hpp"
#include <raymath.h>

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

Texture2D shadowTexture;
void createShadowTexture(){
	Image shadowImage = GenImageGradientLinear(64, 64, 0, ColorAlpha(DARKGRAY, 0.5f), ColorAlpha(WHITE, 0.0f));
	shadowTexture = LoadTextureFromImage(shadowImage);
	UnloadImage(shadowImage);
}

void createShadowsForMap(){
	drawTexture3D(shadowTexture, {0, 1, 1}, WHITE);

	for (const auto& pair : map) {
		for (int x{}; x < CHUNK_SIZE; x++) {
			for (int y{}; y < CHUNK_SIZE; y++) {
				for (int z{}; z < CHUNK_SIZE; z++) {
					if (pair.second.blocks[x][y][z] == AIR) {
						continue;
					}
					
					const Vec3Int chunkWorldPos = pair.first;
					const Vec3Int blockChunkPos = Vec3Int{x, y, z};
					const Vec3Int blockPos = (chunkWorldPos * CHUNK_SIZE) + blockChunkPos;
					const Vec3Int worldPos = ((chunkWorldPos * CHUNK_SIZE) + blockChunkPos) * BLOCK_SIZE;

					Vector3 pos = worldPos.toVec3();
					pos.y = worldPos.y + 0.01f; // epsilon

					if (getBlock(blockPos + Vec3Int{0, 1, -1}) != AIR) {
						drawTexture3DRot(shadowTexture, pos, WHITE, 0);
					}
					if (getBlock(blockPos + Vec3Int{0, 1, 1}) != AIR) {
						drawTexture3DRot(shadowTexture, pos, WHITE, 180);
					}
					if (getBlock(blockPos + Vec3Int{1, 1, 0}) != AIR) {
						drawTexture3DRot(shadowTexture, pos, WHITE, 90);
					}
					if (getBlock(blockPos + Vec3Int{-1, 1, 0}) != AIR) {
						drawTexture3DRot(shadowTexture, pos, WHITE, 270);
					}
				}
			}
		}
	}
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
						const float blockHeight = chunkWorldPos.y * CHUNK_SIZE + blockChunkPos.y;
						const int minBrigtnessDistance = 20;
						const float maxWhite = 0.5f; // lower is more white

						Vector3 whiteBackgroundPos = worldPos.toVec3();
						whiteBackgroundPos.y -= 0.001;
						drawRect3D(whiteBackgroundPos, WHITE);

						float brightness = (blockHeight - player.pos.y); 
						brightness /= minBrigtnessDistance; //(minBrigtnessDistance * 0.01);

						float colorAlpha = 1;
						if (brightness > 0)
							colorAlpha = 1 - brightness;
						if (colorAlpha < maxWhite)
							colorAlpha = maxWhite;
						
						Color c = ColorAlpha(ColorBrightness(WHITE, brightness), colorAlpha);

						drawTexture3D(useTexture("grass.png"), worldPos.toVec3(), c);
					}

					if (pair.second.blocks[x][y][z] == STONE) {
						drawTexture3D(useTexture("stone.png"), worldPos.toVec3(), ColorContrast(WHITE, (player.pos.y - worldPos.y) * 0.1));
					}
				}
			}
		}
	}
}
