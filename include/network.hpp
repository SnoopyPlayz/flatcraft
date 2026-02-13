#pragma once
#include "map.hpp"
#include "enet/enet.h"
#include <vector>

int testNetwork();
void updateNetwork();

// appends the startPtr pointer. size in number of elements not bytes
template <typename T>
std::vector<T> unpackPacket(const ENetPacket& packet, u_int64_t& startPtr, unsigned long size){
	std::vector<T> dataVector;
	dataVector.insert(dataVector.end(), (T*)(packet.data + startPtr), (T*)(packet.data + startPtr) + size);
	startPtr += size * sizeof(T);
	return dataVector;
}

void addToPacketTemp(std::vector<uint8_t>& packetBuffer, void *data, size_t size);

struct BlockUpdatePacket{
	Vec3Int pos;
	Block block;
};

struct ChunkData{
	Chunk chunk;
	Vec3Int pos;
};
