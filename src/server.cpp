#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <enet/enet.h>
#include <enet/types.h>
#include <iostream>
#include "debug.hpp"
#include "player.hpp"
#include "rayUtils.hpp"
#include "network.hpp"
#include "server.hpp"
#include "map.hpp"
#include <stdio.h>
#include <stop_token>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// server globals
ENetAddress address;
ENetHost *server;

std::vector<uint8_t> packetBuffer;
void addToPacketTemp(void *data, size_t size) {
	const uint8_t* byteData = static_cast<const uint8_t*>(data);
	packetBuffer.insert(packetBuffer.end(), byteData, byteData + size);
}

void freePacketTemp() {
	packetBuffer.clear();
}

void updateServer(std::stop_token st) {
	std::unordered_map<ENetPeer *, std::optional<Player>> clients;
	while (!st.stop_requested()) {
	std::vector<PlayerData> playerDataVec;
	std::vector<ChunkData> chunksVec;
	playerDataVec.reserve(clients.size());
	chunksVec.reserve(map.size());

	playerDataVec.push_back({player, 1}); // adding the servers player
	
	// remove nullopt and convert to PlayerData vector
	for (const auto& [uniquePeers, uniquePlayer] : clients) {
		if (!uniquePlayer) continue;
		playerDataVec.push_back({*uniquePlayer, uniquePeers->connectID});
	}
	// convert map to vector of ChunkData
	for (const auto& [pos, chunk] : map){
		chunksVec.push_back({chunk, pos});
	}

	u_int64_t playerDataVecSize = playerDataVec.size();
	u_int64_t chunksVecSize = chunksVec.size();
	// send vector size data
	addToPacketTemp((void *)&playerDataVecSize, sizeof(u_int64_t));
	addToPacketTemp((void *)&chunksVecSize, sizeof(u_int64_t));
	// send vector data
	addToPacketTemp(playerDataVec.data(), playerDataVec.size() * sizeof(PlayerData));
	addToPacketTemp(chunksVec.data(), chunksVec.size() * sizeof(ChunkData));

	for (const auto& [peer, clientP] : clients) {
		if (!clientP) {// this may not be needed TODO FIXME
			std::cout << "skipping client with id: " << peer->connectID << " because it has no player data" << std::endl;
		}
		// draw players for server client
		drawTexture3D(useTexture("player.png"), clientP->pos, WHITE);

		ENetPacket* packet = enet_packet_create(packetBuffer.data(), packetBuffer.size(), ENET_PACKET_FLAG_UNSEQUENCED);
		enet_peer_send(peer, 0, packet);
	}
	freePacketTemp();

	ENetEvent event;
	while (enet_host_service(server, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("A client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
				clients[event.peer] = std::nullopt;
				break;
			case ENET_EVENT_TYPE_RECEIVE: {
				// set player in the map
				auto it = clients.find(event.peer);
				assert(it != clients.end());
				it->second = *(Player*)event.packet->data;

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
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}

	// send a disconnect packet to all clients
	for (const auto& [peer, clientP] : clients) {
		enet_peer_disconnect(peer, 0);
	}
	enet_host_flush(server);
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
