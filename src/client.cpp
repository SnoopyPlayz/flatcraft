#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <enet/enet.h>
#include <stdio.h>
#include <vector>
#include "player.hpp"
#include "rayUtils.hpp"

// client globals
ENetHost *client;
ENetPeer *peer;

void updateClient() {
	static std::vector<Player> players;
	ENetEvent event;

	while (enet_host_service(client, &event, 10) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("connected to server from %x:%u.\n", 
						event.peer->address.host, 
						event.peer->address.port);
				break;
			case ENET_EVENT_TYPE_RECEIVE:{
				players.clear();
				Player * p = (Player*)event.packet->data;

				for (unsigned long i = {0}; i < event.packet->dataLength / sizeof(Player); i++) {
					players.push_back(p[i]);
				}

				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", static_cast<char *>(event.peer->data));
				players.clear();
				break;

			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}
	for (Player p : players) {
		drawTexture3D(useTexture("player.png"), p.pos, WHITE);
	}
	ENetPacket *packet = enet_packet_create(static_cast<void *>(&player), sizeof(player), ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(peer, 0, packet);
}

bool createClient() {
	// 1 outgoing connection 2 channels any amount of bandwidth
	client = enet_host_create(NULL, 1, 2, 0, 0);

	if (client == NULL) {
		fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
		return false;
	}

	ENetAddress address;
	ENetEvent event;

	/* Connect to some.server.net:1234. */
	enet_address_set_host(&address, "localhost");
	address.port = 1234;

	/* Initiate the connection, allocating the two channels 0 and 1. */
	peer = enet_host_connect(client, &address, 2, 0);

	if (peer == NULL) {
		fprintf(stderr, "No available peers for initiating an ENet connection.\n");
		return false;
	}

	/* Wait up to 5 seconds for the connection attempt to succeed. */
	if (enet_host_service(client, &event, 5000) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT) {
		puts("Connection to localhost succeeded.");
	} else {
		/* Either the 5 seconds are up or a disconnect event was */
		/* received. Reset the peer in the event the 5 seconds   */
		/* had run out without any significant event.            */
		enet_peer_reset(peer);
		enet_host_destroy(client);

		puts("Connection to localhost failed.");
		return false;
	}
	return true;
}
