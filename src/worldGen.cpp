#include <cassert>
#include <cstdio>
#include <iostream>
#include <mutex>
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
	if (findOrCreateChunk(pos).generated == false) {
		if (pos == Vec3Int{3, -1, -5}) {
			printf("setting block in not generated chunk: %d %d %d\n", chunkPos.z, chunkPos.y, chunkPos.z);
			std::cout << "invalid block position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
			assert(!IsKeyDown(KEY_R));
		}
		setBlock(pos, block);
		findOrCreateChunk(pos).generated = true;
	}
}


void genChunk(Vec3Int chunkPos){
	assert(findOrCreateChunk(chunkPos).generated == false);
	Vec3Int worldPos = chunkPos * CHUNK_SIZE;

	for(int x = worldPos.x; x < CHUNK_SIZE; x++){
		for(int z = worldPos.z; z < CHUNK_SIZE; z++){
			int height = (int)(noise.GetNoise((float)x,(float)z) * 10);

			if(height < worldPos.y || height > worldPos.y + CHUNK_SIZE){
				continue;
			}

			setBlockInNotGeneratedChunk({x, height, z}, GRASS);

			for(int i = height - 1; i > 0; i--){
				setBlockInNotGeneratedChunk({x, height, z}, GRASS);
			}
		}
	}
	findOrCreateChunk(chunkPos).generated = true;
}
