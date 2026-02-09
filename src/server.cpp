#include <algorithm>
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
	static std::vector<ENetPeer *> players;
	ENetEvent event;
	void *data = (void *)"Client information";

	while (enet_host_service(server, &event, 10) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %x:%u.\n", event.peer->address.host,
						event.peer->address.port);
				clients[event.peer] = std::nullopt;
				/* Store any relevant client information here. */
				event.peer->data = data;

				break;
			case ENET_EVENT_TYPE_RECEIVE: {
				printf("\n SERVER: A packet of length %zu containing was received from %s on channel %u.\n",
						event.packet->dataLength, event.packet->data, 
						event.channelID);

				auto it = clients.find(event.peer);
				assert(it != clients.end());
				it->second = *(Player*)event.packet->data;

				printf("\n pos %f \n", it->second->pos.x);

				// use packet here
				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", static_cast<char *>(event.peer->data));

				/* Reset the peer's client information. */

				event.peer->data = NULL;
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}

	
	for (const auto& [peer, player] : clients) {
		drawTexture3D(useTexture("player.png"), player->pos, WHITE);

		// send all players to the player other than its self
		std::vector<Player> uniquePlayers;

		for (const auto& [uniquePeers, uniquePlayer] : clients) {
			if (peer == uniquePeers || peer == nullptr || uniquePlayer == std::nullopt) 
				continue;

			std::cout << "sending player pos: " << uniquePlayer->pos.x << std::endl;
			uniquePlayers.push_back(uniquePlayer.value());
		}
		ENetPacket * packet = enet_packet_create (uniquePlayers.data(), uniquePlayers.size() * sizeof(Player), ENET_PACKET_FLAG_RELIABLE);

		enet_peer_send(peer, 0, packet);
	}
}

int hostServer() {
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	server = enet_host_create(&address, 32, 2, 0, 0);

	if (server == NULL) {
		fprintf(stderr,
				"An error occurred while trying to create an ENet server host.\n");
		exit(EXIT_FAILURE);
	}
	printf("ENet server host created successfully\n");
	return 0;
}
