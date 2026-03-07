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

void createChunk(Vec3Int pos){
	std::lock_guard<std::mutex> lock(mapMtx);
	auto result = map.emplace(std::make_pair(pos, Chunk{}));
	assert(result.second);
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
	Chunk& c = findChunk(chunkPos);
	std::lock_guard<std::mutex> lock(mapMtx);
	c.blocks[localChunkPos.x][localChunkPos.y][localChunkPos.z] = block;
	c.changed = true;
}

Texture2D shadowTexture;
void createShadowTexture(){
	Image shadowImage = GenImageGradientLinear(64, 64, 0, ColorAlpha(DARKGRAY, 0.5f), ColorAlpha(WHITE, 0.0f));
	shadowTexture = LoadTextureFromImage(shadowImage);
	UnloadImage(shadowImage);
}

bool culling(Vec3Int pos){
	const int radius = 1;
	Vec3Int position = toVec3Int(player.pos) / CHUNK_SIZE;

	/*for (int x = position.x - radius; x <= radius; x++) {
		for (int y = position.y - radius; y <= radius; y++) {
			for (int z = position.z - radius; z <= radius; z++) {

				if(position == (Vec3Int){x + pos.x,y + pos.y,z + pos.z}){
					return true;
				}
			}
		}
	}*/

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
	for (const auto& pair : map) {
		if(!culling(pair.first)){
			continue;
		}

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

void debugMap(){
	if (!debug.enabled) {
		return;
	}
	for (const auto& pair : map) {
		Vec3Int pos = pair.first;
		drawTextSDF(std::to_string(pos.x) + " " + std::to_string(pos.z), pos.x * CHUNK_SIZE * BLOCK_SIZE, pos.z * CHUNK_SIZE * BLOCK_SIZE, 50, RED);
		drawTextSDF(std::to_string(pos.x) + " " + std::to_string(pos.z), pos.x * CHUNK_SIZE * BLOCK_SIZE, pos.z * CHUNK_SIZE * BLOCK_SIZE, 51, GRAY);
	}
}

void drawMap(){
	for (const auto& pair : map) {
		if(!culling(pair.first)){
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
					const Vec3Int worldPos = ((chunkWorldPos * CHUNK_SIZE) + blockChunkPos) * BLOCK_SIZE;
					const float blockHeight = chunkWorldPos.y * CHUNK_SIZE + blockChunkPos.y;

					// draw white background tile
					Vector3 whiteBackgroundPos = worldPos.toVec3();
					whiteBackgroundPos.y -= 0.001;
					drawRect3D(whiteBackgroundPos, WHITE);

					// compute brightness and transparency of the tile
					float brightness = (blockHeight - player.pos.y); 
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

					drawTexture3D(useTexture(nameStr + ".png"), worldPos.toVec3(), c);
				}
			}
		}
	}
}
