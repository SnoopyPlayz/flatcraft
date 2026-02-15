#pragma once
#include "map.hpp"
#include "enet/enet.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <string>
#include <vector>

int initNetwork();
void configureNetwork(bool useInternalServer, const std::string& host, uint16_t port);
void updateNetwork();
void shutdownNetwork();

// appends the startPtr pointer. size in number of elements not bytes
template <typename T>
std::vector<T> unpackPacket(const ENetPacket& packet, uint64_t& startPtr, unsigned long size){
	std::vector<T> dataVector;
	//std::memcpy(dataVector.data(), packet.data + startPtr, (sizeof(T) * (size_t)((packet.data + startPtr) + size)));
	dataVector.insert(dataVector.end(), (T*)(packet.data + startPtr), (T*)(packet.data + startPtr) + size);
	startPtr += size * sizeof(T);
	return dataVector;
}

template <typename T>
std::vector<T> unpackVecFromPacket(const ENetPacket& packet, uint64_t& startPtr){
	uint64_t vecSize = unpackPacket<uint64_t>(packet, startPtr, 1)[0];
	if(vecSize == 0) return std::vector<T>();
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

#pragma pack(push, 1)
struct BlockUpdatePacket{
	Vec3Int pos;
	Block block;
};

struct ChunkData{
	Chunk chunk;
	Vec3Int pos;
};
#pragma pack(pop)
