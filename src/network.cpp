#include "server.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <enet/enet.h>
#include <sys/types.h>
#include <thread>
#include "client.hpp"
#include "network.hpp"
#include <string>
#include <vector>

int initNetwork(){
	if (enet_initialize () != 0){
		fprintf (stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	printf("ENet initialized successfully\n");
	return 0;
}

std::jthread serverThread;
std::jthread clientThread;
bool networkingStarted = false;
bool useInternalServer = true;
std::string connectHost = "localhost";
uint16_t connectPort = 1236;

void configureNetwork(bool shouldUseInternalServer, const std::string& host, uint16_t port) {
	useInternalServer = shouldUseInternalServer;
	connectHost = host;
	connectPort = port;
}

static void startSinglePlayerNetworking() {
	if (networkingStarted) return;
	if (useInternalServer && !hostServer(connectPort)) return;

	if (useInternalServer) {
		serverThread = std::jthread(updateServer);
	}
	if (!createClient(connectHost.c_str(), connectPort)) {
		if (useInternalServer) {
			serverThread.request_stop();
			if (serverThread.joinable()) serverThread.join();
		}
		return;
	}

	clientThread = std::jthread(updateClient);
	networkingStarted = true;
}

void updateNetwork(){
	startSinglePlayerNetworking();
	if (networkingStarted) drawClients();
}

void shutdownNetwork() {
	if (clientThread.joinable()) {
		clientThread.request_stop();
		clientThread.join();
	}
	if (serverThread.joinable()) {
		serverThread.request_stop();
		serverThread.join();
	}
	networkingStarted = false;
}

void addToPacketTemp(std::vector<uint8_t>& packetBuffer, void *data, size_t size) {
	const uint8_t* byteData = static_cast<const uint8_t*>(data);
	packetBuffer.insert(packetBuffer.end(), byteData, byteData + size);
}
