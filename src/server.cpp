#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <enet/enet.h>
#include <enet/types.h>
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

// server globals
ENetAddress address;
ENetHost *server;

void addToPacketForEachPeer(std::vector<uint8_t>& packetBuffer, ENetPeer* peer, std::unordered_map<ENetPeer *, std::vector<Vec3Int>>& chunkRequests, std::vector<BlockUpdatePacket> blockUpdatesVec, const std::optional<Player>& player) {
	std::cout << "using receved packet form peer" << peer << std::endl;
	std::cout << "adding chunks for peer with id: " << chunkRequests[peer].size() << std::endl;
	std::vector<ChunkData> chunksVec;
	for (Vec3Int &chunkPos: chunkRequests[peer]) {
		if (validChunk(chunkPos)) {
			chunksVec.push_back({findChunk(chunkPos), chunkPos});
			std::cout << "added chunk: " << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << std::endl;
		}
	}
	addVecToPacket(packetBuffer, chunksVec);
	for (const BlockUpdatePacket& blockUpdate : blockUpdatesVec) {
		Vec3Int chunkBlockUpdatePos = blockUpdate.pos / CHUNK_SIZE;
		if (Vec3Int{1,1,1} < (toVec3Int(player->pos) / CHUNK_SIZE) - chunkBlockUpdatePos){
			std::cout << "skipping block update at pos: " << blockUpdate.pos.x << " " << blockUpdate.pos.y << " " << blockUpdate.pos.z << std::endl;
		}
	}

	addVecToPacket(packetBuffer, blockUpdatesVec);
	chunkRequests[peer].clear();
}

void networkTick(std::unordered_map<ENetPeer *, std::optional<Player>>& clients) {
	static std::unordered_map<ENetPeer *, std::vector<Vec3Int>> chunkRequests;
	static std::vector<BlockUpdatePacket> blockUpdatesVec;
	std::vector<PlayerData> playerDataVec;
	std::vector<ChunkData> chunksVec;
	playerDataVec.reserve(clients.size());
	chunksVec.reserve(map.size());

	playerDataVec.push_back({player, 1}); // adding the servers player
	
	// convert map to vector and remove player = nullopt
	for (const auto& [uniquePeers, uniquePlayer] : clients) {
		if (!uniquePlayer){
			std::cout << "unique player skipped" << std::endl;
			continue;
		}
		playerDataVec.push_back({*uniquePlayer, uniquePeers->connectID});
	}
	// convert map to vector of ChunkData
	for (const auto& [pos, chunk] : map){
		chunksVec.push_back({chunk, pos});
	}

	std::vector<uint8_t> packetBuffer; // the packet that will be sent

	addVecToPacket(packetBuffer, playerDataVec);
	//addVecToPacket(packetBuffer, chunksVec);

	for (const auto& [peer, clientP] : clients) {
		if (!clientP) {// this may not be needed TODO FIXME
			std::cout << "skipping client with id: " << peer->connectID << " because it has no player data" << std::endl;
		}

		addToPacketForEachPeer(packetBuffer, peer, chunkRequests, blockUpdatesVec, clientP);

		// draw players for server client
		drawTexture3D(useTexture("player.png"), clientP->pos * BLOCK_SIZE, WHITE);

		ENetPacket* packet = enet_packet_create(packetBuffer.data(), packetBuffer.size(), ENET_PACKET_FLAG_UNSEQUENCED);
		enet_peer_send(peer, 0, packet);
	}
	blockUpdatesVec.clear();

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
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}

	// send a disconnect packet to all clients
	for (const auto& [peer, clientP] : clients) {
		enet_peer_disconnect(peer, 0);
	}
	enet_host_flush(server);
	enet_host_destroy(server);
}

bool hostServer() {
	address.host = ENET_HOST_ANY;
	address.port = 1236;

	server = enet_host_create(&address, 32/*maxClients*/, 2/*maxChannels*/, 0/*incomingBandwidth*/, 0/*outgoingBandwidth*/);

	if (server == NULL) {
		fprintf(stderr, "Cant create Enet server \n");
		return false;
	}

	if (enet_host_compress_with_range_coder(server) < 0){
		fprintf(stderr, "Cant set compression \n");
		return false;
	}
	printf("server created successfully\n");
	return true;
}
