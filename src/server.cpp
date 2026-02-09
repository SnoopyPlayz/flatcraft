#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <enet/enet.h>
#include <iostream>
#include "player.hpp"
#include "rayUtils.hpp"
#include "server.hpp"
#include <stdio.h>
#include <unordered_map>
#include <vector>

// server globals
ENetAddress address;
ENetHost *server;
std::unordered_map<ENetPeer *, std::optional<Player>> clients;

void updateServer() {
	ENetEvent event;

	while (enet_host_service(server, &event, 10) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("A client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
				clients[event.peer] = std::nullopt;
				break;
			case ENET_EVENT_TYPE_RECEIVE: {
				printf("\n SERVER: A packet length: %zu channel: %u.\n",
						event.packet->dataLength, 
						event.channelID);
				// set player in the map
				auto it = clients.find(event.peer);
				assert(it != clients.end());
				it->second = *(Player*)event.packet->data;

				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s client disconnected.\n", static_cast<char *>(event.peer->data));
				clients.erase(event.peer);
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}

	
	for (const auto& [peer, clientP] : clients) {
		// draw players for server client
		drawTexture3D(useTexture("player.png"), clientP->pos, WHITE);

		// send all players to the player other than its self
		std::vector<Player> uniquePlayers;
		for (const auto& [uniquePeers, uniquePlayer] : clients) {
			if (peer == uniquePeers || peer == nullptr || uniquePlayer == std::nullopt) 
				continue;

			std::cout << "sending player pos: " << uniquePlayer->pos.x << std::endl;
			uniquePlayers.push_back(uniquePlayer.value());
		}

		uniquePlayers.push_back(player); // adding the servers player

		ENetPacket * packet = enet_packet_create (uniquePlayers.data(), uniquePlayers.size() * sizeof(Player), ENET_PACKET_FLAG_RELIABLE);

		enet_peer_send(peer, 0, packet);
	}
}

bool hostServer() {
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	server = enet_host_create(&address, 32, 2, 0, 0);

	if (server == NULL) {
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		return false;
	}
	printf("ENet server host created successfully\n");
	return true;
}
