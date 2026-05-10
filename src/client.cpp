#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>
#include <mutex>
#include <lz4.h>
#include <stdio.h>
#include <stop_token>
#include <string>
#include <sys/types.h>
#include <unordered_set>
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
		
	for (size_t i{}; i < oldPlayers.size(); i++) {
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
				const Vector3 drawPosWorld = drawPos * BLOCK_SIZE;
				const Texture2D playerTexture = useTexture("player.png");
				queueDraw3D(
					drawPosWorld.y,
					[playerTexture, drawPosWorld]() {
						DrawTextureWithRot(playerTexture, drawPosWorld.x, drawPosWorld.z, 0, WHITE, 1.0f);
					}
				);

				// draw breaking overlay for other players
				if (player.player.blockBreakingProgress > 0.0f) {
					const Vector3 breakWorldPos = player.player.blockBreakingPos.toVec3() * (float)BLOCK_SIZE;
					const Texture2D breakTex = useTexture("block_breaking.png");
					const float alpha = player.player.blockBreakingProgress;
					queueDraw3D(breakWorldPos.y + 0.01,
						[breakTex, breakWorldPos, alpha]() {
							DrawTextureWithRot(breakTex, breakWorldPos.x, breakWorldPos.z, 0, ColorAlpha(WHITE, alpha), 1.0f);
						});
				}

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

std::vector<uint8_t> packetBuffer; // the packet that will be sent
std::mutex packetBufferMtx;
void preparePacket(){
	std::vector<Vec3Int> chunkRequests;

	const int radius = 1;
	RADUIS(radius){
		Vec3Int chunkpos = (toVec3Int(player.pos) / CHUNK_SIZE) + Vec3Int{x, y, z};
		if (!map.validChunk(chunkpos)) {
			//Vector3 debugRect = chunkpos.toVec3() * CHUNK_SIZE * BLOCK_SIZE;
			//debugRect.y += 10;
			//queueDraw3D(debugRect.y, [debugRect]() { DrawRectangle((int)debugRect.x, (int)debugRect.z, BLOCK_SIZE, BLOCK_SIZE, RED); });
			chunkRequests.push_back(chunkpos);
		}
	}

	std::lock_guard<std::mutex> lock(packetBufferMtx);
	// if packet not sent yet by client thread
	if (packetBuffer.size() != 0) {
		return;
	}
	addVariableToPacket(packetBuffer, player);
	addVecToPacket(packetBuffer, blockUpdates);
	blockUpdates.clear();
	addVecToPacket(packetBuffer, chunkRequests);
	addVecToPacket(packetBuffer, inventoryMoves);
	inventoryMoves.clear();
}


std::vector<ENetPacket*> recevedPacketVec;
std::mutex recevedPacketMtx;

void clearQueuedReceivedPackets(){
	std::lock_guard<std::mutex> lock(recevedPacketMtx);
	for (ENetPacket* packet : recevedPacketVec) {
		if (packet != nullptr) {
			enet_packet_destroy(packet);
		}
	}
	recevedPacketVec.clear();
}

void processRecevedPacket(){
	std::lock_guard<std::mutex> lock(recevedPacketMtx);
	for (ENetPacket* packet : recevedPacketVec) {
		uint64_t ptrPos = 0;

		players.clear();
		players = unpackVecFromPacket<PlayerData>(*packet, ptrPos);

		// player struct from server
		for (const PlayerData& pd : players) {
		        if (pd.peer == peer->connectID) {
		                memcpy(player.inventory, pd.player.inventory, sizeof(player.inventory));
		                player.health = pd.player.health;
		                //player.selectedBlock = pd.player.selectedBlock;
		                break;
		        }
		}

		std::vector<ChunkData> chunks = unpackCompressedVecFromPacket<ChunkData>(*packet, ptrPos);

		for (const ChunkData& chunkData : chunks) {
			map.findOrCreateChunk(chunkData.pos) = std::move(chunkData.chunk);
			map.markShadowAffectedChunksChanged(chunkData.pos);
		}

		std::vector<BlockUpdatePacket> blockUpdate = unpackVecFromPacket<BlockUpdatePacket>(*packet, ptrPos);
		for (const BlockUpdatePacket& blockUpdatePacket : blockUpdate) {
			if (map.validChunk(blockUpdatePacket.pos / CHUNK_SIZE)) {
				map.setBlock(blockUpdatePacket.pos, blockUpdatePacket.block);
			}
		}

		std::vector<Item> serverItems = unpackVecFromPacket<Item>(*packet, ptrPos);
		{
                        // process the sent items from server
			std::unordered_set<uint32_t> serverIds;
			for (const Item& si : serverItems) {
				serverIds.insert(si.id);
                                // find item from server
				auto it = std::find_if(items.begin(), items.end(),
					[&](const Item& li) { return li.id == si.id; });

				if (it != items.end()) {
					it->pos = si.pos;
				} else {
					items.push_back(si);
				}
			}
			// remove items not in server list (picked up or despawned)
			std::erase_if(items, [&](const Item& li) {
				return !serverIds.contains(li.id);
			});
		}

		enet_packet_destroy(packet);
	}
	recevedPacketVec.clear();
}

void updateClients(){
}

bool netowrkTick(){
	ENetEvent event;

	{
		std::lock_guard<std::mutex> lock(packetBufferMtx);
		if (!packetBuffer.empty()) {
			ENetPacket *packet = enet_packet_create(packetBuffer.data(), packetBuffer.size(), ENET_PACKET_FLAG_RELIABLE);
			packetBuffer.clear();
			if (enet_peer_send(peer, 0, packet) < 0) {
				enet_packet_destroy(packet);
			}
		}
	}

	while (enet_host_service(client, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("connected to server from %x:%u.\n", 
						event.peer->address.host, 
						event.peer->address.port);
				break;
			case ENET_EVENT_TYPE_RECEIVE:{
				std::lock_guard<std::mutex> lock(recevedPacketMtx);
				recevedPacketVec.push_back(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				puts("disconnected from server");
				players.clear();
				clearQueuedReceivedPackets();
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
	clearQueuedReceivedPackets();
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
		enet_host_destroy(client);
		return false;
	}

	const int connectTimeoutMs = 5000;
	const int connectPollMs = 100;
	for (int elapsed = 0; elapsed < connectTimeoutMs; elapsed += connectPollMs) {
		const int serviceResult = enet_host_service(client, &event, connectPollMs);
		if (serviceResult < 0) {
			break;
		}
		if (serviceResult == 0) {
			continue;
		}

		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				puts("Connection to localhost succeeded.");
				return true;
			case ENET_EVENT_TYPE_RECEIVE: {
				std::lock_guard<std::mutex> lock(recevedPacketMtx);
				recevedPacketVec.push_back(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				enet_peer_reset(peer);
				enet_host_destroy(client);
				clearQueuedReceivedPackets();
				puts("Connection to localhost failed.");
				return false;
			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}

	enet_peer_reset(peer);
	enet_host_destroy(client);
	clearQueuedReceivedPackets();
	puts("Connection to localhost failed.");
	return false;
}
