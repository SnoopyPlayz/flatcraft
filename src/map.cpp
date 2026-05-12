#include "map.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <FastNoiseLite.h>
#include <raylib.h>
#include <raymath.h>
#include <magic_enum.hpp>

#include "debug.hpp"
#include "player.hpp"
#include "rayUtils.hpp"
#include "vector.hpp"

Map map;

struct CachedShadowChunk {
	std::vector<Texture2DInstance> vertices;
	bool ready = false;
};

std::map<Vec3Int, CachedShadowChunk> mapShadows;
Texture2D shadowTexture;

static bool culling(Vec3Int pos, Vec3Int center, const int radius) {
	Vec3Int position = center / CHUNK_SIZE;

	RADUIS(radius) {
		if (position == pos + Vec3Int{x, y, z}) {
			return true;
		}
	}
	return false;
}

void Map::createChunk(Vec3Int pos) {
	auto result = chunks.emplace(std::make_pair(pos, Chunk{}));
	assert(result.second);
}

void Map::markShadowAffectedChunksChanged(Vec3Int chunkPos) {
	RADUIS(1) {
		auto it = chunks.find(chunkPos + Vec3Int{x, y, z});
		if (it == chunks.end()) {
			continue;
		}
		it->second.changed = true;
	}
}

Chunk& Map::findChunk(Vec3Int pos) {
	auto it = chunks.find(pos);
	assert(it != chunks.end() && "cant find chunk");
	return it->second;
}

Chunk& Map::findOrCreateChunk(Vec3Int pos) {
	auto it = chunks.find(pos);
	if (it == chunks.end()) {
		auto [newIt, inserted] = chunks.emplace(std::make_pair(pos, Chunk{}));
		assert(inserted);
		return newIt->second;
	}
	return it->second;
}

bool Map::validChunk(Vec3Int pos) {
	return chunks.find(pos) != chunks.end();
}

Block Map::getBlock(Vec3Int pos) {
	Vec3Int chunkPos = pos / CHUNK_SIZE;
	Vec3Int localChunkPos = pos.mod(CHUNK_SIZE);

	auto it = chunks.find(chunkPos);
	if (it == chunks.end()) {
		return AIR;
	}
	return static_cast<Block>(it->second.blocks[localChunkPos.x][localChunkPos.y][localChunkPos.z]);
}

std::optional<Vec3Int> Map::findTopBlock(int x, int y) {
	for (int i = MAX_BLOCK_SEARCH_HEIGHT; i > -MAX_BLOCK_SEARCH_HEIGHT; i--) {
		if (getBlock({x, i, y}) != AIR) {
			return Vec3Int{x, i, y};
		}
	}
	return std::nullopt;
}

void Map::setBlock(Vec3Int pos, Block block) {
	Vec3Int chunkPos = pos / CHUNK_SIZE;
	Vec3Int localChunkPos = pos.mod(CHUNK_SIZE);

	auto [it, inserted] = chunks.emplace(std::make_pair(chunkPos, Chunk{}));
	(void)inserted;
	it->second.blocks[localChunkPos.x][localChunkPos.y][localChunkPos.z] = static_cast<uint8_t>(block);

	RADUIS(1) {
		auto affectedIt = chunks.find(chunkPos + Vec3Int{x, y, z});
		if (affectedIt == chunks.end()) {
			continue;
		}
		affectedIt->second.changed = true;
	}
}


void Map::createShadowTexture() {
	Image shadowImage = GenImageGradientLinear(64, 64, 0, ColorAlpha(DARKGRAY, 0.5f), ColorAlpha(WHITE, 0.0f));
	shadowTexture = LoadTextureFromImage(shadowImage);
	UnloadImage(shadowImage);
}

void Map::createShadowsForMap() {
	std::vector<Vec3Int> rebuiltChunks;
	int rebuiltTiles = 0;

	for (const auto& pair : chunks) {
		if (!culling(pair.first, toVec3Int(player.pos), 1)) {
			continue;
		}

		CachedShadowChunk& cachedShadowChunk = mapShadows[pair.first];
		if (cachedShadowChunk.ready && !pair.second.changed) {
			drawTexture3DInstances(cachedShadowChunk.vertices);
			continue;
		}

		cachedShadowChunk.vertices.clear();

		FOR_XYZ(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE) {
			if (pair.second.blocks[x][y][z] == AIR) {
				continue;
			}

			const Vec3Int chunkWorldPos = pair.first;
			const Vec3Int blockChunkPos = Vec3Int{x, y, z};
			const Vec3Int blockPos = (chunkWorldPos * CHUNK_SIZE) + blockChunkPos;
			const Vec3Int worldPos = blockPos * BLOCK_SIZE;

			Vector3 pos = worldPos.toVec3();
			pos.y = worldPos.y + 0.01f; // epsilon

			auto hasSolid = [this](const Vec3Int& p) {
				Vec3Int neighborChunkPos = p / CHUNK_SIZE;
				Vec3Int neighborLocalPos = p.mod(CHUNK_SIZE);
				auto it = chunks.find(neighborChunkPos);
				if (it == chunks.end()) {
					return false;
				}
				return it->second.blocks[neighborLocalPos.x][neighborLocalPos.y][neighborLocalPos.z] != AIR;
			};
			if (hasSolid(blockPos + Vec3Int{0, 1, 0})) {
				continue;
			}

			rebuiltTiles++;
			auto pushShadow = [&cachedShadowChunk, pos](float rotation) {
				cachedShadowChunk.vertices.push_back({
					pos.y,
					[pos, rotation, shadowTex = shadowTexture]() {
						DrawTextureWithRot(shadowTex, pos.x, pos.z, rotation, WHITE, 1.0f);
					}
				});
			};

			if (hasSolid(blockPos + Vec3Int{0, 1, -1})) {
				pushShadow(0);
			}
			if (hasSolid(blockPos + Vec3Int{0, 1, 1})) {
				pushShadow(180);
			}
			if (hasSolid(blockPos + Vec3Int{1, 1, 0})) {
				pushShadow(90);
			}
			if (hasSolid(blockPos + Vec3Int{-1, 1, 0})) {
				pushShadow(270);
			}
		}

		cachedShadowChunk.ready = true;
		drawTexture3DInstances(cachedShadowChunk.vertices);
		rebuiltChunks.push_back(pair.first);
	}

	if (rebuiltTiles > 0) {
		std::cout << "rebuilt shadow chunk " + std::to_string(rebuiltTiles) << std::endl;
	}

	for (const Vec3Int& chunkPos : rebuiltChunks) {
		auto it = chunks.find(chunkPos);
		if (it == chunks.end()) {
			continue;
		}
		it->second.changed = false;
	}
}

void Map::debugMap() {
	if (!debug.enabled) {
		return;
	}

	for (const auto& pair : chunks) {
		Vec3Int pos = pair.first;
		if (culling(pos, toVec3Int(player.pos), 1)) {
			drawTextSDF3D(
				std::to_string(pos.x) + " " + std::to_string(pos.z),
				pos.x * CHUNK_SIZE * BLOCK_SIZE,
				pos.z * CHUNK_SIZE * BLOCK_SIZE,
				50,
				RED
			);
		}
	}
}

void Map::drawMap() {
	for (const auto& pair : chunks) {
		if (!culling(pair.first, toVec3Int(player.pos), 1)) {
			continue;
		}

		FOR_XYZ(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE) {
			const Block currentBlock = static_cast<Block>(pair.second.blocks[x][y][z]);
			if (currentBlock == AIR) {
				continue;
			}

			const int minBrigtnessDistance = 20;
			const float maxWhite = 0.5f; // lower is more white

			const Vec3Int chunkWorldPos = pair.first;
			const Vec3Int blockChunkPos = Vec3Int{x, y, z};
			const Vec3Int worldPixelPos = ((chunkWorldPos * CHUNK_SIZE) + blockChunkPos) * BLOCK_SIZE;
			const Vec3Int worldPos = (chunkWorldPos * CHUNK_SIZE) + blockChunkPos;

			Vec3Int topPos = worldPos + Vec3Int(0, 1, 0);
			Vec3Int topChunkPos = topPos / CHUNK_SIZE;
			Vec3Int topLocalPos = topPos.mod(CHUNK_SIZE);
			auto topChunkIt = chunks.find(topChunkPos);

			uint8_t blockAboveThisOne = topChunkIt->second.blocks[topLocalPos.x][topLocalPos.y][topLocalPos.z];
			if (topChunkIt != chunks.end() && blockAboveThisOne != AIR && blockAboveThisOne != LEAVES) {
				continue;
			}

			Vector3 whiteBackgroundPos = worldPixelPos.toVec3();
			whiteBackgroundPos.y -= 0.001f;
			if (currentBlock != LEAVES){
				queueDraw3D(
					whiteBackgroundPos.y,
					[whiteBackgroundPos]() {
						DrawRectangle((int)whiteBackgroundPos.x, (int)whiteBackgroundPos.z, BLOCK_SIZE, BLOCK_SIZE, WHITE);
					}
				);
			}

			float brightness = (worldPos.y - player.pos.y);
			brightness /= minBrigtnessDistance;

			float colorAlpha = 1;
			if (brightness > 0) {
				colorAlpha = 1 - brightness;
			}
			if (colorAlpha < maxWhite) {
				colorAlpha = maxWhite;
			}

			Color c = ColorAlpha(ColorBrightness(WHITE, brightness), colorAlpha);
			const Vector3 worldPixelPos3D = worldPixelPos.toVec3();
			const Texture2D blockTexture = useTexture(getEnumName(currentBlock) + ".png");
			queueDraw3D(
				worldPixelPos3D.y,
				[blockTexture, worldPixelPos3D, c]() {
					DrawTextureWithRot(blockTexture, worldPixelPos3D.x, worldPixelPos3D.z, 0, c, 1.0f);
				}
			);
		}
	}
}
