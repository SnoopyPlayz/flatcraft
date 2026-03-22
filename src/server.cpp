#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "player.hpp"
#include "rayUtils.hpp"
#include "network.hpp"
#include "server.hpp"
#include "map.hpp"
#include "vector.hpp"
#include <stdio.h>
#include <stop_token>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <raymath.h>
#include <stdio.h>
#include <lz4.h>
#include "worldGen.hpp"
#include "item.hpp"

// server globals
ENetAddress address;
ENetHost *server;

template<typename T>
void addCompressedVecToPacket(std::vector<uint8_t>& packetBuffer, const std::vector<T>& dataVec) {
	const uint64_t uncompressedSize = dataVec.size();
	addVariableToPacket(packetBuffer, uncompressedSize);

	int srcSize = static_cast<int>(dataVec.size() * sizeof(T));
	int maxDstSize = LZ4_compressBound(srcSize);
	std::vector<uint8_t> compressedData;
	compressedData.resize(maxDstSize);

	int compressedSize = LZ4_compress_default(
			(const char *)dataVec.data(),             // Source pointer
			(char *)compressedData.data(),      // Destination pointer
			srcSize,             // Source size
			maxDstSize// Max destination capacity
	);
	assert(compressedSize > 0 || srcSize == 0);
	compressedData.resize(compressedSize);

	addVecToPacket(packetBuffer, compressedData);
}


void addToPacketForEachPeer(std::vector<uint8_t>& packetBuffer, const std::vector<Vec3Int>& peerChunkRequests, const std::vector<BlockUpdatePacket>& blockUpdatesVec, const std::vector<Item>& itemsVec) {
	std::vector<ChunkData> chunksVec;
	for (const Vec3Int& chunkPos: peerChunkRequests) {
		std::cout << "chunk received: " << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << std::endl;
		Chunk& c = findOrCreateChunk(chunkPos);
		if (!c.generated){// this is torture
			genChunk(chunkPos);
		}
		chunksVec.push_back({findOrCreateChunk(chunkPos), chunkPos});
	}
	addCompressedVecToPacket(packetBuffer, chunksVec);
	addVecToPacket(packetBuffer, blockUpdatesVec);
	addVecToPacket(packetBuffer, itemsVec);
}

void networkTick(std::unordered_map<ENetPeer *, std::optional<Player>>& clients) {
	static std::unordered_map<ENetPeer *, std::vector<Vec3Int>> chunkRequests;
	static std::vector<BlockUpdatePacket> blockUpdatesVec;
	static std::vector<Item> itemsVec;
	std::vector<PlayerData> playerDataVec;
	playerDataVec.reserve(clients.size());

	// convert map to vector and remove player = nullopt
	for (const auto& [uniquePeers, uniquePlayer] : clients) {
		if (!uniquePlayer){
			std::cout << "unique player skipped" << std::endl;
			continue;
		}
		playerDataVec.push_back({*uniquePlayer, uniquePeers->connectID});
	}

	const std::vector<Vec3Int> emptyChunkRequests;

	for (const auto& [peer, clientP] : clients) {
		if (!clientP) {// this may not be needed TODO FIXME
			std::cout << "skipping client with id: " << peer->connectID << " because it has no player data" << std::endl;
		}

		std::vector<uint8_t> packetBuffer; // the packet that will be sent
		addVecToPacket(packetBuffer, playerDataVec);

		const auto requestIt = chunkRequests.find(peer);
		const std::vector<Vec3Int>& peerChunkRequests = requestIt != chunkRequests.end() ? requestIt->second : emptyChunkRequests;
		addToPacketForEachPeer(packetBuffer, peerChunkRequests, blockUpdatesVec, itemsVec);

		ENetPacket* packet = enet_packet_create(packetBuffer.data(), packetBuffer.size(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
	}
	chunkRequests.clear();
	blockUpdatesVec.clear();
	itemsVec.clear();

	ENetEvent event;
	while (enet_host_service(server, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("A client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
				clients[event.peer] = std::nullopt;
				break;
			case ENET_EVENT_TYPE_RECEIVE: {
				// set player in the map
				uint64_t ptrPos = 0;
				Player player = unpackPacket<Player>(*(event.packet), ptrPos, 1)[0];

				std::vector<BlockUpdatePacket> blockUpdates = unpackVecFromPacket<BlockUpdatePacket>(*(event.packet), ptrPos);

				auto it = clients.find(event.peer);
				assert(it != clients.end());
				it->second = player;

				for (const BlockUpdatePacket& blockUpdate : blockUpdates) {
					if (blockUpdate.dropItem != AIR) {
						dropItem(blockUpdate.dropItem, (blockUpdate.pos * BLOCK_SIZE).toVec3());
						itemsVec.push_back({blockUpdate.dropItem, (blockUpdate.pos * BLOCK_SIZE).toVec3()});
					}
					setBlock(blockUpdate.pos, blockUpdate.block);
					blockUpdatesVec.push_back(blockUpdate);
				}

				chunkRequests[event.peer] = unpackVecFromPacket<Vec3Int>(*(event.packet), ptrPos);

				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				puts("disconnected from server");
				clients.erase(event.peer);
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}
}

void updateServer(std::stop_token st) {
	std::unordered_map<ENetPeer *, std::optional<Player>> clients;
	while (!st.stop_requested()) {
		networkTick(clients);
		//std::this_thread::sleep_for(std::chrono::milliseconds(30));
		WaitTime(0.03);
	}

	// send a disconnect packet to all clients
	for (const auto& [peer, clientP] : clients) {
		enet_peer_disconnect(peer, 0);
	}
	enet_host_flush(server);
	enet_host_destroy(server);
}

bool hostServer(uint16_t port) {
	address.host = ENET_HOST_ANY;
	address.port = port;

	server = enet_host_create(&address, 32/*maxClients*/, 2/*maxChannels*/, 0/*incomingBandwidth*/, 0/*outgoingBandwidth*/);

	if (server == NULL) {
		fprintf(stderr, "Cant create Enet server \n");
		return false;
	}

	printf("server created successfully\n");
	return true;
}
