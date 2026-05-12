#include "map.hpp"
#include "worldGen.hpp"
#include <FastNoiseLite.h>

thread_local FastNoiseLite noise;
thread_local bool noiseReady = false;

void Map::worldGenInit() {
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noiseReady = true;
}

void Map::genChunk(Vec3Int chunkPos) {
	if (!noiseReady) {
		worldGenInit();
	}

	Chunk& targetChunk = findOrCreateChunk(chunkPos);
	if (targetChunk.generated) {
		return;
	}

	Vec3Int worldPos = chunkPos * CHUNK_SIZE;
	for (int x = worldPos.x; x < worldPos.x + CHUNK_SIZE; x++) {
		for (int z = worldPos.z; z < worldPos.z + CHUNK_SIZE; z++) {
			int height = static_cast<int>((noise.GetNoise(static_cast<float>(x), static_cast<float>(z)) + 5) * 4);

			if (height < worldPos.y || height >= worldPos.y + CHUNK_SIZE) {
				if (height <= worldPos.y) {
					continue;
				}
				for (int y = CHUNK_SIZE; y > 0; y--) {
					Vec3Int stonePos{x, y, z};
					Vec3Int stoneChunkPos = stonePos / CHUNK_SIZE;
					Chunk& stoneChunk = findOrCreateChunk(stoneChunkPos);
					if (!stoneChunk.generated) {
						setBlock(stonePos, STONE);
					}
				}
				continue;
			}

			Vec3Int topPos{x, height, z};
			Vec3Int topChunkPos = topPos / CHUNK_SIZE;
			Chunk& topChunk = findOrCreateChunk(topChunkPos);
			if (!topChunk.generated) {
				setBlock(topPos, GRASS);
			}

			for (int y = height - 1; y > 0; y--) {
				Vec3Int stonePos{x, y, z};
				Vec3Int stoneChunkPos = stonePos / CHUNK_SIZE;
				Chunk& stoneChunk = findOrCreateChunk(stoneChunkPos);
				if (!stoneChunk.generated) {
					setBlock(stonePos, STONE);
				}
			}
			if (GetRandomValue(0,64) == 0){
				genTree(topPos);
			}
		}
	}

	findOrCreateChunk(chunkPos).generated = true;
}

void Map::genTree(Vec3Int treePos){
	Vec3Int positions[] = {{0,0,0}, {0,1,0}, {0,2,0}};
	for(Vec3Int pos : positions){
		setBlock(treePos + pos, WOOD);
	}

	RADUIS(1){
		setBlock({x + treePos.x, treePos.y + 2, z + treePos.z}, LEAVES);
	}
	setBlock({treePos.x, treePos.y + 2, treePos.z}, WOOD);
}
