#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <enet/enet.h>
#include <iostream>
#include <stdio.h>
#include <stop_token>
#include <sys/types.h>
#include <utility>
#include <vector>
#include "map.hpp"
#include "network.hpp"
#include "player.hpp"
#include "rayUtils.hpp"

// client globals
ENetHost *client;
ENetPeer *peer;

std::vector<PlayerData> players;
std::mutex playersMtx;

void drawClients(){
	std::lock_guard<std::mutex> lock(playersMtx);
	for (const PlayerData& playerClient : players) {
		if (playerClient.peer == peer->connectID) {
			continue;
		}
		drawTexture3D(useTexture("player.png"), playerClient.player.pos, WHITE);
	}
}

void updateClient(std::stop_token st) {
	while (!st.stop_requested()) {
	ENetEvent event;

	while (enet_host_service(client, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("connected to server from %x:%u.\n", 
						event.peer->address.host, 
						event.peer->address.port);
				break;
			case ENET_EVENT_TYPE_RECEIVE:{
				unsigned long ptrPos = 0;
				// get the amount of player and chunks in the packet
				u_int64_t playersSize = unpackPacket<u_int64_t>(*(event.packet), ptrPos, 1)[0];
				u_int64_t chunkSize = unpackPacket<u_int64_t>(*(event.packet), ptrPos, 1)[0];

				{
					std::lock_guard<std::mutex> lock(playersMtx);
					players.clear();
					players = unpackPacket<PlayerData>(*(event.packet), ptrPos, playersSize);
				}

				std::vector<ChunkData> chunks = unpackPacket<ChunkData>(*(event.packet), ptrPos, chunkSize);

				for (const ChunkData& chunkData : chunks) {
					findOrCreateChunk(chunkData.pos) = std::move(chunkData.chunk);
				}

				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				puts("disconnected from server");
				players.clear();
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}

	std::vector<uint8_t> packetBuffer; // the packet that will be sent
	u_int64_t blockUpdatesSize = blockUpdates.size();
	addToPacketTemp(packetBuffer, (void *)&blockUpdatesSize, sizeof(u_int64_t));
	addToPacketTemp(packetBuffer, (void *)&player, sizeof(player));
	addToPacketTemp(packetBuffer, blockUpdates.data(), blockUpdates.size() * sizeof(BlockUpdatePacket));

	ENetPacket *packet = enet_packet_create(packetBuffer.data(), packetBuffer.size(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	// send a disconnect packet to the server
	enet_peer_disconnect(peer, 0);
	enet_host_flush(client);
}

bool createClient() {
	client = enet_host_create(NULL, 1/*maxClients*/, 2/*maxChannels*/, 0/*incomingBandwidth*/, 0/*outgoingBandwidth*/);

	if (client == NULL) {
		fprintf(stderr, "Cant create Enet host\n");
		return false;
	}

	ENetAddress address;
	ENetEvent event;

	// connect to localhost
	enet_address_set_host(&address, "localhost");
	address.port = 1236;

	peer = enet_host_connect(client, &address, 1/*chanel amount*/, 0/*data to give host*/);

	if (peer == NULL) {
		fprintf(stderr, "No available peers for initiating an ENet connection.\n");
		return false;
	}
	if (enet_host_compress_with_range_coder(client) < 0) {
		fprintf(stderr, "Cant set compression \n");
		return false;
	}

	// 5 sec timeout for connection
	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		puts("Connection to localhost succeeded.");
	} else {
		enet_peer_reset(peer);
		enet_host_destroy(client);
		puts("Connection to localhost failed.");
		return false;
	}
	return true;
}
