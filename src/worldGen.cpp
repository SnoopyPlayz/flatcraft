#include "map.hpp"
#include "worldGen.hpp"

void worldGenInit() {
	map.worldGenInit();
}

void genChunk(Vec3Int chunkPos) {
	map.genChunk(chunkPos);
}
