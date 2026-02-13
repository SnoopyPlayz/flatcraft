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

template <typename T>
std::vector<T> unpackVecFromPacket(const ENetPacket& packet, u_int64_t& startPtr){
	u_int64_t vecSize = unpackPacket<u_int64_t>(packet, startPtr, 1)[0];
	std::vector<T> result = unpackPacket<T>(packet, startPtr, vecSize);

	return result;
}

void addToPacketTemp(std::vector<uint8_t>& packetBuffer, void *data, size_t size);

template<typename T>
void addVecToPacket(std::vector<uint8_t>& packetBuffer, std::vector<T>& vector) {
	const uint64_t vectorSize = vector.size();
	addToPacketTemp(packetBuffer, (void *)&vectorSize, sizeof(uint64_t));
	addToPacketTemp(packetBuffer, vector.data(), sizeof(T) * vector.size());
}

template<typename T>
void addVariableToPacket(std::vector<uint8_t>& packetBuffer, const T &data) {
	addToPacketTemp(packetBuffer, (void *)&data, sizeof(T));
}

struct BlockUpdatePacket{
	Vec3Int pos;
	Block block;
};

struct ChunkData{
	Chunk chunk;
	Vec3Int pos;
};
