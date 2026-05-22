#include "map.hpp"
#include "worldGen.hpp"
#include <FastNoiseLite.h>
#include <cstdio>

thread_local FastNoiseLite noise;
thread_local bool noiseReady = false;

void Map::worldGenInit() {
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetSeed(100);
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
			float noiseHeight = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
                        int height = static_cast<int>((noiseHeight + 5) * 4);

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

		}
	}

	for (int x = worldPos.x; x < worldPos.x + CHUNK_SIZE; x++) {
		for (int z = worldPos.z; z < worldPos.z + CHUNK_SIZE; z++) {
			float noiseHeight = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
                        int height = static_cast<int>((noiseHeight + 5) * 4);
			Vec3Int topPos{x, height, z};

                        noise.SetSeed(500);
			float treeHeight = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));

			if (GetRandomValue(0,32) == 0 && treeHeight > 0.7f) {
				genTree(topPos);
			}

                        noise.SetSeed(400);
			float sandPatchPos = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));

			if (GetRandomValue(0,2) == 0 && sandPatchPos > 0.90f) {
				genPatch(topPos, SAND);
			}

                        noise.SetSeed(200);
			float gravelPatchPos = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));

			if (GetRandomValue(0,2) == 0 && gravelPatchPos > 0.92f) {
				genPatch(topPos, GRAVEL);
			}

                        noise.SetSeed(100);

                }
        }
	findOrCreateChunk(chunkPos).generated = true;
}

void Map::genPatch(Vec3Int sandPos, Block blockType){
        RADUIS(2){
                setBlock({x + sandPos.x, sandPos.y, z + sandPos.z}, blockType);
        }
}

void Map::genTree(Vec3Int treePos){
	Vec3Int positions[] = {{0,0,0}, {0,1,0}, {0,2,0}};
	for(Vec3Int pos : positions){
		setBlock(treePos + pos, WOOD);
	}

	RADUIS(1){
                // below leaves has to be air
                if (getBlock({x + treePos.x, treePos.y + 1, z + treePos.z}) != AIR || getBlock({x + treePos.x, treePos.y + 2, z + treePos.z}) != AIR) {
                        continue;
                
                }
		setBlock({x + treePos.x, treePos.y + 2, z + treePos.z}, LEAVES);
	}
	setBlock({treePos.x, treePos.y + 2, treePos.z}, WOOD);
}
