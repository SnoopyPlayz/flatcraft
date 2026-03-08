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
#include "vector.hpp"
#include <raymath.h> 
#include <iostream>
#include <magic_enum.hpp>

std::map<Vec3Int, Chunk> map;
std::mutex mapMtx;

struct CachedShadowChunk {
	std::vector<Texture2DInstance> vertices;
	bool ready = false;
};

std::map<Vec3Int, CachedShadowChunk> mapShadows;

void createChunk(Vec3Int pos){
	std::lock_guard<std::mutex> lock(mapMtx);
	auto result = map.emplace(std::make_pair(pos, Chunk{}));
	assert(result.second);
}

void markShadowAffectedChunksChanged(Vec3Int chunkPos){
	std::lock_guard<std::mutex> lock(mapMtx);
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 0; y++) {
			for (int z = -1; z <= 1; z++) {
				auto it = map.find(chunkPos + Vec3Int{x, y, z});
				if (it == map.end()) {
					continue;
				}
				it->second.changed = true;
			}
		}
	}
}

Chunk& findChunk(Vec3Int pos){
	std::lock_guard<std::mutex> lock(mapMtx);
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
	std::lock_guard<std::mutex> lock(mapMtx);
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
	std::lock_guard<std::mutex> lock(mapMtx);
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

	std::lock_guard<std::mutex> lock(mapMtx);
	map[chunkPos].blocks[localChunkPos.x][localChunkPos.y][localChunkPos.z] = block;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 0; y++) {
			for (int z = -1; z <= 1; z++) {
				auto it = map.find(chunkPos + Vec3Int{x, y, z});
				if (it == map.end()) {
					continue;
				}
				it->second.changed = true;
			}
		}
	}
}

Texture2D shadowTexture;
void createShadowTexture(){
	Image shadowImage = GenImageGradientLinear(64, 64, 0, ColorAlpha(DARKGRAY, 0.5f), ColorAlpha(WHITE, 0.0f));
	shadowTexture = LoadTextureFromImage(shadowImage);
	UnloadImage(shadowImage);
}

bool culling(Vec3Int pos, Vec3Int center, const int radius){
	Vec3Int position = center / CHUNK_SIZE;

	for (int x = -radius; x <= radius; x++) {
		for (int y = -radius; y <= radius; y++) {
			for (int z = -radius; z <= radius; z++) {

				if (position == pos + Vec3Int{x,y,z}) {
					return true;
				}
			}
		}
	}
	return false;
}

void createShadowsForMap(){
	std::vector<Vec3Int> rebuiltChunks;

	for (const auto& pair : map) {
		if(!culling(pair.first, toVec3Int(player.pos), 1)){
			continue;
		}

		CachedShadowChunk& cachedShadowChunk = mapShadows[pair.first];
		if (cachedShadowChunk.ready && !pair.second.changed) {
			drawTexture3DInstances(cachedShadowChunk.vertices);
			continue;
		}

		cachedShadowChunk.vertices.clear();

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
						cachedShadowChunk.vertices.push_back({{pos.x, pos.y, pos.z}, shadowTexture, WHITE, 0});
					}
					if (getBlock(blockPos + Vec3Int{0, 1, 1}) != AIR) {
						cachedShadowChunk.vertices.push_back({{pos.x, pos.y, pos.z}, shadowTexture, WHITE, 180});
					}
					if (getBlock(blockPos + Vec3Int{1, 1, 0}) != AIR) {
						cachedShadowChunk.vertices.push_back({{pos.x, pos.y, pos.z}, shadowTexture, WHITE, 90});
					}
					if (getBlock(blockPos + Vec3Int{-1, 1, 0}) != AIR) {
						cachedShadowChunk.vertices.push_back({{pos.x, pos.y, pos.z}, shadowTexture, WHITE, 270});
					}
				}
			}
		}

		cachedShadowChunk.ready = true;
		drawTexture3DInstances(cachedShadowChunk.vertices);
		rebuiltChunks.push_back(pair.first);
	}

	std::lock_guard<std::mutex> lock(mapMtx);
	for (const Vec3Int& chunkPos : rebuiltChunks) {
		auto it = map.find(chunkPos);
		if (it == map.end()) {
			continue;
		}
		it->second.changed = false;
	}
}

void debugMap(){
	if (!debug.enabled) {
		return;
	}
	for (const auto& pair : map) {
		Vec3Int pos = pair.first;
		if(culling(pos, toVec3Int(player.pos), 1)){
			drawTextSDF(std::to_string(pos.x) + " " + std::to_string(pos.z), pos.x * CHUNK_SIZE * BLOCK_SIZE, pos.z * CHUNK_SIZE * BLOCK_SIZE, 50, RED);
		}
	}
}

void drawMap(){
	for (const auto& pair : map) {
		if(!culling(pair.first, toVec3Int(player.pos), 1)){
			continue;
		}

		for (int x{}; x < CHUNK_SIZE; x++) {
			for (int y{}; y < CHUNK_SIZE; y++) {
				for (int z{}; z < CHUNK_SIZE; z++) {
					const Block currentBlock = (Block)pair.second.blocks[x][y][z];
					if (currentBlock == AIR) {
						continue;
					}

					const int minBrigtnessDistance = 20;
					const float maxWhite = 0.5f; // lower is more white

					// assuming that there is a block at 34 0 0
					// 1 0 0 chunk position
					const Vec3Int chunkWorldPos = pair.first;
					// 2 0 0 block position in chunk
					const Vec3Int blockChunkPos = Vec3Int{x, y, z};

					// chunk world pos * size of chunk + block chunk pos * pixel size of block
					// 	     1 0 0 * 32 	   + 2 0 0   	     * 64 = 2176 0 0
					// 1 0 0 * 32 + 2 0 0 * 64 = 2176 0 0
					const Vec3Int worldPixelPos = ((chunkWorldPos * CHUNK_SIZE) + blockChunkPos) * BLOCK_SIZE;
					const Vec3Int worldPos = ((chunkWorldPos * CHUNK_SIZE) + blockChunkPos);

					// culling
					if (getBlock(worldPos + Vec3Int(0, 1, 0)) != AIR) {
						continue;
					}

					// draw white background tile
					Vector3 whiteBackgroundPos = worldPixelPos.toVec3();
					whiteBackgroundPos.y -= 0.001;
					drawRect3D(whiteBackgroundPos, WHITE);

					// compute brightness and transparency of the tile
					float brightness = (worldPos.y - player.pos.y); 
					brightness /= minBrigtnessDistance; //(minBrigtnessDistance * 0.01);

					float colorAlpha = 1;
					if (brightness > 0)
						colorAlpha = 1 - brightness;
					if (colorAlpha < maxWhite)
						colorAlpha = maxWhite;

					Color c = ColorAlpha(ColorBrightness(WHITE, brightness), colorAlpha);
					// get name of texture
					auto enumName = magic_enum::enum_name(currentBlock);
					std::string nameStr { enumName };
					nameStr = stringToLower(nameStr);

					drawTexture3D(useTexture(nameStr + ".png"), worldPixelPos.toVec3(), c);
				}
			}
		}
	}
}
