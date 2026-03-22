#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>
#include <lz4.h>
#include <stdio.h>
#include <stop_token>
#include <sys/types.h>
#include <utility>
#include <vector>
#include "debug.hpp"
#include "item.hpp"
#include "map.hpp"
#include "network.hpp"
#include "player.hpp"
#include "rayUtils.hpp"
#include "raymath.h"
#include "vector.hpp"

// client globals
ENetHost *client;
ENetPeer *peer;

std::vector<PlayerData> players;
std::mutex playersMtx;

template<typename T>
std::vector<T> unpackCompressedVecFromPacket(const ENetPacket& packet, uint64_t& startPtr) {
	const uint64_t uncompressedSize = unpackPacket<uint64_t>(packet, startPtr, 1)[0];
	std::vector<uint8_t> compressedData = unpackVecFromPacket<uint8_t>(packet, startPtr);
	if (uncompressedSize == 0) return {};

	std::vector<T> decompressedData(uncompressedSize);
	const int dstSize = static_cast<int>(uncompressedSize * sizeof(T));
	const int decompressedSize = LZ4_decompress_safe(
		(const char*)compressedData.data(),
		(char*)decompressedData.data(),
		static_cast<int>(compressedData.size()),
		dstSize
	);
	assert(decompressedSize == dstSize);
	return decompressedData;
}


void drawClients(){
	const float lerpAmount = 0.1; // lower = less stutter
	static std::vector<PlayerData> oldPlayers;
	std::lock_guard<std::mutex> lock(playersMtx);
		
	for (unsigned long i{}; i < oldPlayers.size(); i++) {
		bool playerExists = false;
		for(PlayerData& player : players){
			if (player.peer == oldPlayers[i].peer){
				playerExists = true;
			}
		}
		if(!playerExists){
			oldPlayers[i] = oldPlayers.back();
			oldPlayers.pop_back();
		}
	}

	for(PlayerData& player : players){
		if (player.peer == peer->connectID) {
			continue;
		}
		bool playerExists = false;
		for (PlayerData& oldPlayer : oldPlayers) {
			if (player.peer == oldPlayer.peer){
				playerExists = true;
				Player &p = oldPlayer.player;
				Vector3 drawPos = p.pos ;
				drawPos.x -= 0.5;
				drawPos.y += 0.01;
				drawPos.z -= 0.5;
				drawTexture3D(useTexture("player.png"), drawPos* BLOCK_SIZE, WHITE);

				p.pos += player.player.velocity;
				p.pos = Vector3Lerp(p.pos, player.player.pos, lerpAmount);
				if (player.player.velocity == (Vector3){0,0,0}) {
					p.pos = player.player.pos;
				}
				continue;
			}
		}
		if (!playerExists){
			oldPlayers.push_back(player);
		}
	}
}

void updateClients(){
	std::lock_guard<std::mutex> lock(playersMtx);
	for(PlayerData& player : players){
		pickUpItems(player.player.pos);
	}
}

bool netowrkTick(){
	ENetEvent event;
	std::vector<Vec3Int> chunkRequests;

	const int radius = 1;
	RADUIS(radius){
		Vec3Int chunkpos = (toVec3Int(player.pos) / CHUNK_SIZE) + Vec3Int{x, y, z};
		if (!validChunk(chunkpos)) {
			//Vector3 debugRect = chunkpos.toVec3() * CHUNK_SIZE * BLOCK_SIZE;
			//debugRect.y += 10;
			//drawRect3D(debugRect, RED);
			chunkRequests.push_back(chunkpos);
		}
	}

	std::vector<uint8_t> packetBuffer; // the packet that will be sent
	addVariableToPacket(packetBuffer, player);
	addVecToPacket(packetBuffer, blockUpdates);
	{
		std::lock_guard<std::mutex> lock(blockUpdateMutex);
		addVecToPacket(packetBuffer, chunkRequests);
	}
	blockUpdates.clear();

	ENetPacket *packet = enet_packet_create(packetBuffer.data(), packetBuffer.size(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);

	while (enet_host_service(client, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("connected to server from %x:%u.\n", 
						event.peer->address.host, 
						event.peer->address.port);
				break;
			case ENET_EVENT_TYPE_RECEIVE:{
				uint64_t ptrPos = 0;

				{
					std::lock_guard<std::mutex> lock(playersMtx);
					players.clear();
					players = unpackVecFromPacket<PlayerData>(*(event.packet), ptrPos);
				}

				std::vector<ChunkData> chunks = unpackCompressedVecFromPacket<ChunkData>(*(event.packet), ptrPos);

				for (const ChunkData& chunkData : chunks) {
					findOrCreateChunk(chunkData.pos) = std::move(chunkData.chunk);
					markShadowAffectedChunksChanged(chunkData.pos);
				}

				std::vector<BlockUpdatePacket> blockUpdate = unpackVecFromPacket<BlockUpdatePacket>(*(event.packet), ptrPos);
				for (const BlockUpdatePacket& blockUpdatePacket : blockUpdate) {
					if (validChunk(blockUpdatePacket.pos / CHUNK_SIZE)) {
						setBlock(blockUpdatePacket.pos, blockUpdatePacket.block);
					}
				}

				std::vector<Item> item = unpackVecFromPacket<Item>(*(event.packet), ptrPos);
				for (Item i : item) {
					dropItem(i.b, i.pos);
				}

				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				puts("disconnected from server");
				players.clear();
				return false;
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}
	return true;
}

void updateClient(std::stop_token st) {
	while (!st.stop_requested() && netowrkTick()){
		//std::this_thread::sleep_for(std::chrono::milliseconds(30));
		WaitTime(0.03);
	}
	enet_peer_disconnect(peer, 0);
	enet_host_flush(client);
	enet_host_destroy(client);
}

bool createClient(const char* host, uint16_t port) {
	client = enet_host_create(NULL, 1/*maxClients*/, 2/*maxChannels*/, 0/*incomingBandwidth*/, 0/*outgoingBandwidth*/);

	if (client == NULL) {
		fprintf(stderr, "Cant create Enet host\n");
		return false;
	}

	ENetAddress address;
	ENetEvent event;

	enet_address_set_host(&address, host);
	address.port = port;

	peer = enet_host_connect(client, &address, 1/*chanel amount*/, 0/*data to give host*/);

	if (peer == NULL) {
		fprintf(stderr, "No available peers for initiating an ENet connection.\n");
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
