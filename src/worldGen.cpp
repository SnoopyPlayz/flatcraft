#include <cassert>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <raylib.h>
#include <stdio.h>
#include <FastNoiseLite.h>
#include "vector.hpp"
#include "map.hpp"

FastNoiseLite noise;
void worldGenInit(){
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

void setBlockInNotGeneratedChunk(Vec3Int pos, Block block){
	Vec3Int chunkPos = pos / CHUNK_SIZE;
	if (findOrCreateChunk(chunkPos).generated == false) {
		setBlock(pos, block);
	}
}


void genChunk(Vec3Int chunkPos){
	assert(findOrCreateChunk(chunkPos).generated == false);
	Vec3Int worldPos = chunkPos * CHUNK_SIZE;

	for(int x = worldPos.x; x < worldPos.x + CHUNK_SIZE; x++){
		for(int z = worldPos.z; z < worldPos.z + CHUNK_SIZE; z++){
			int height = (int)((noise.GetNoise((float)x,(float)z) + 1) * 10);

			if(height < worldPos.y || height >= worldPos.y + CHUNK_SIZE){
				continue;
			}

			setBlockInNotGeneratedChunk({x, height, z}, GRASS);

			for(int i = height - 1; i > 0; i--){
				setBlockInNotGeneratedChunk({x, i, z}, STONE);
			}
		}
	}
	findOrCreateChunk(chunkPos).generated = true;
}
