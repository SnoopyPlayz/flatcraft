#include <iostream>
#include <FastNoiseLite.h>
#include "vector.hpp"
#include "map.hpp"

FastNoiseLite noise;
void worldGenInit(){
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}


void genChunk(Vec3Int chunkPos){
	Vec3Int worldPos = chunkPos * CHUNK_SIZE;
	findOrCreateChunk(chunkPos).generated = true;

	for(int x = worldPos.x; x < CHUNK_SIZE; x++){
		for(int z = worldPos.z; z < CHUNK_SIZE; z++){
			int height = (int)(noise.GetNoise((float)x,(float)z) * 10);

			if(height < worldPos.y || height > worldPos.y + CHUNK_SIZE){
				continue;
			}

			setBlock({x, height, z}, GRASS);

			for(int i = height - 1; i > 0; i--){
				setBlock({x, i, z}, STONE);
			}
		}
	}
}
