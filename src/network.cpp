#include "server.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <enet/enet.h>
#include <thread>
#include "client.hpp"
#include <raylib.h>

int testNetwork(){
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

	if (IsKeyPressed(KEY_L)) {
		if (hostServer()){
			networkThread = std::jthread(updateServer);
		}
	}

	if (IsKeyPressed(KEY_K)) {
		if (createClient()) {
			networkThread = std::jthread(updateClient);
		}
	}
}
