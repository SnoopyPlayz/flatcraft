#include "server.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <enet/enet.h>
#include <sys/types.h>
#include <thread>
#include "client.hpp"
namespace rl {
    // We suppress warnings because Raylib (C library) might
    // trigger some C++ warnings when inside a namespace.
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    
    #include "raylib.h"

    #pragma GCC diagnostic pop
}
#include <vector>

int initNetwork(){
	if (enet_initialize () != 0){
		fprintf (stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	printf("ENet initialized successfully\n");
	return 0;
}

std::jthread networkThread;
void updateNetwork(){
	drawClients();

	if (rl::IsKeyPressed(rl::KEY_L)) {
		if (hostServer()){
			networkThread = std::jthread(updateServer);
		}
	}

	if (rl::IsKeyPressed(rl::KEY_K)) {
		if (createClient()) {
			networkThread = std::jthread(updateClient);
		}
	}
}

void addToPacketTemp(std::vector<uint8_t>& packetBuffer, void *data, size_t size) {
	const uint8_t* byteData = static_cast<const uint8_t*>(data);
	packetBuffer.insert(packetBuffer.end(), byteData, byteData + size);
}
