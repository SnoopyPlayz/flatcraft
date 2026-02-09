#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <enet/enet.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include "player.hpp"
#include "rayUtils.hpp"

// client globals
ENetHost *client;
ENetPeer *peer;

void updateClient() {
	ENetEvent event;
	static std::vector<Player> players;
	/* Wait up to 1000 milliseconds for an event. */
	void *data = (void *)"Client information";
	while (enet_host_service(client, &event, 10) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %x:%u.\n", 
						event.peer->address.host, 
						event.peer->address.port);

				/* Store any relevant client information here. */
				event.peer->data = data;

				break;
			case ENET_EVENT_TYPE_RECEIVE:{
				printf("\n CLIENT: A packet of length %zu containing was received "
						"from on channel %u.\n",
						event.packet->dataLength, 
						event.channelID);

				players.clear();
				Player * p = (Player*)event.packet->data;

				for (unsigned long i = {0}; i < event.packet->dataLength / sizeof(Player); i++) {
					players.push_back(p[i]);
				}

				/* Clean up the packet now that we're done using it. */
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
	for (Player p : players) {
		drawTexture3D(useTexture("player.png"), p.pos, WHITE);
	}
	ENetPacket *packet = enet_packet_create(static_cast<void *>(&player), sizeof(player), ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(peer, 0, packet);
}

void createClient() {
	client = enet_host_create(NULL /* create a client host */,
			1 /* only allow 1 outgoing connection */,
			2 /* allow up 2 channels to be used, 0 and 1 */,
			0 /* assume any amount of incoming bandwidth */,
			0 /* assume any amount of outgoing bandwidth */);

	if (client == NULL) {
		fprintf(stderr,
				"An error occurred while trying to create an ENet client host.\n");
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
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

		puts("Connection to some.server.net:1234 failed.");
	}
}
