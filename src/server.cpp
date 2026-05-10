#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "player.hpp"
#include "rayUtils.hpp"
#include "network.hpp"
#include "server.hpp"
#include "map.hpp"
#include "vector.hpp"
#include <stdio.h>
#include <stop_token>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <raymath.h>
#include <stdio.h>
#include <lz4.h>
#include "item.hpp"
#include "crafting.hpp"

// server globals
ENetAddress address;
ENetHost *server;
Map serverMap;
static uint32_t nextItemId = 0;

template<typename T>
static void addCompressedVecToPacket(std::vector<uint8_t>& packetBuffer, const std::vector<T>& dataVec) {
	const uint64_t uncompressedSize = dataVec.size();
	addVariableToPacket(packetBuffer, uncompressedSize);

	int srcSize = static_cast<int>(dataVec.size() * sizeof(T));
	int maxDstSize = LZ4_compressBound(srcSize);
	std::vector<uint8_t> compressedData;
	compressedData.resize(maxDstSize);

	int compressedSize = LZ4_compress_default(
			(const char *)dataVec.data(),// Source pointer
			(char *)compressedData.data(),// Destination pointer
			srcSize,// Source size
			maxDstSize// Max destination capacity
	);
	assert(compressedSize > 0 || srcSize == 0);
	compressedData.resize(compressedSize);

	addVecToPacket(packetBuffer, compressedData);
}


static void addToPacketForEachPeer(std::vector<uint8_t>& packetBuffer, const std::vector<Vec3Int>& peerChunkRequests, const std::vector<BlockUpdatePacket>& blockUpdatesVec, const std::vector<Item>& itemsVec) {
	std::vector<ChunkData> chunksVec;
	for (const Vec3Int& chunkPos: peerChunkRequests) {
		std::cout << "chunk received: " << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << std::endl;
		Chunk& c = serverMap.findOrCreateChunk(chunkPos);
		if (!c.generated){// this is torture
			serverMap.genChunk(chunkPos);
		}
		chunksVec.push_back({serverMap.findOrCreateChunk(chunkPos), chunkPos});
	}
	addCompressedVecToPacket(packetBuffer, chunksVec);
	addVecToPacket(packetBuffer, blockUpdatesVec);
	addVecToPacket(packetBuffer, itemsVec);
}

static void tickItemPhysics(std::vector<Item>& itemsVec) {
	for (Item& item : itemsVec) {
		item.velocity.y -= 0.3f;
		item.pos.y += item.velocity.y;
		Vec3Int blockUnder = toVec3Int(item.pos / (float)BLOCK_SIZE);
		if (serverMap.getBlock(blockUnder) != AIR) {
			item.pos.y = (float)(blockUnder.y + 1) * BLOCK_SIZE;
			item.velocity = {0, 0, 0};
		}
	}
}

static void tickItemPickup(std::vector<Item>& itemsVec, std::unordered_map<ENetPeer*, std::optional<Player>>& clients) {
	std::erase_if(itemsVec, [&](const Item& item) {
		for (auto& [peer, clientP] : clients) {
			if (!clientP) continue;
			Player& p = *clientP;
			Vector3 itemCenter = item.pos;
			itemCenter.x += (float)BLOCK_SIZE / 2.0f;
			itemCenter.z += (float)BLOCK_SIZE / 2.0f;
			float dist = Vector3Distance(itemCenter, p.pos * (float)BLOCK_SIZE);
			if (dist < (float)BLOCK_SIZE * 1.5f) {
				for (int slot = 0; slot < PLAYER_INVENTORY_SIZE; slot++) {
					if (p.inventory[slot] == AIR) {
						p.inventory[slot] = (uint8_t)item.b;
						return true;
					}
				}
			}
		}
		return false;
	});
}

static std::vector<PlayerData> buildPlayerDataVec(const std::unordered_map<ENetPeer*, std::optional<Player>>& clients) {
	std::vector<PlayerData> playerDataVec;
	playerDataVec.reserve(clients.size());
	for (const auto& [peer, playerOpt] : clients) {
		if (!playerOpt) {
			std::cout << "unique player skipped" << std::endl;
			continue;
		}
		playerDataVec.push_back({*playerOpt, peer->connectID});
	}
	return playerDataVec;
}

static void handleReceivePacket(
    ENetPacket* packet,
    ENetPeer* peer,
    std::unordered_map<ENetPeer*, std::optional<Player>>& clients,
    std::unordered_map<ENetPeer*, std::vector<Vec3Int>>& chunkRequests,
    std::vector<BlockUpdatePacket>& blockUpdatesVec,
    std::vector<Item>& itemsVec) {
	if (packet == nullptr || packet->dataLength == 0) {
		if (packet != nullptr)
			enet_packet_destroy(packet);
		assert(false && "Received empty packet from client");
		return;
	}

	uint64_t ptrPos = 0;
	std::vector<Player> players = unpackPacket<Player>(*packet, ptrPos, 1);
	if (players.empty()) {
		enet_packet_destroy(packet);
		assert(false && "Received malformed player packet from client");
		return;
	}
	Player player = players[0];

	std::vector<BlockUpdatePacket> blockUpdates = unpackVecFromPacket<BlockUpdatePacket>(*packet, ptrPos);

	auto it = clients.find(peer);
	assert(it != clients.end());
	if (!it->second) {
		it->second = player;
	} else {
		it->second->pos = player.pos;
		it->second->velocity = player.velocity;
		it->second->selectedBlock = player.selectedBlock;
		it->second->selectedSlot = player.selectedSlot;
		it->second->health = player.health;
		it->second->blockBreakingPos = player.blockBreakingPos;
		it->second->blockBreakingProgress = player.blockBreakingProgress;
	}

	for (const BlockUpdatePacket& blockUpdate : blockUpdates) {
		if (blockUpdate.block == AIR) {
			Block droppedBlock = serverMap.getBlock(blockUpdate.pos);
			if (droppedBlock != AIR) {
				Vector3 worldPos = blockUpdate.pos.toVec3() * (float)BLOCK_SIZE;
				itemsVec.push_back({nextItemId++, droppedBlock, worldPos, {0, 0, 0}});
			}
		} else {
			Player& p = *it->second;
			if (p.selectedSlot >= PLAYER_INVENTORY_SIZE ||
			    p.inventory[p.selectedSlot] != (uint8_t)blockUpdate.block) {
				blockUpdatesVec.push_back({blockUpdate.pos, AIR});
				continue;
			}
			p.inventory[p.selectedSlot] = AIR;
		}
		serverMap.setBlock(blockUpdate.pos, blockUpdate.block);
		blockUpdatesVec.push_back(blockUpdate);
	}

	chunkRequests[peer] = unpackVecFromPacket<Vec3Int>(*packet, ptrPos);

	std::vector<InventoryMovePacket> inventoryMoves =
	    unpackVecFromPacket<InventoryMovePacket>(*packet, ptrPos);
	for (const auto& move : inventoryMoves) {
		if (move.fromSlot == SLOT_CRAFT_OUTPUT && move.toSlot < PLAYER_INVENTORY_SIZE) {
			if (it->second->craftingResult != AIR && it->second->inventory[move.toSlot] == AIR) {
				it->second->inventory[move.toSlot] = (uint8_t)it->second->craftingResult;
				for (int i = 0; i < CRAFTING_GRID_SIZE; i++)
					it->second->craftingSlots[i] = AIR;
				it->second->craftingResult = AIR;
			}
			continue;
		}

		auto getSlotPtr = [&](uint8_t slot) -> uint8_t* {
			if (slot < PLAYER_INVENTORY_SIZE) return &it->second->inventory[slot];
			if (slot >= SLOT_CRAFT_OFFSET && slot < SLOT_CRAFT_OUTPUT)
				return &it->second->craftingSlots[slot - SLOT_CRAFT_OFFSET];
			return nullptr;
		};

		uint8_t* from = getSlotPtr(move.fromSlot);
		uint8_t* to = getSlotPtr(move.toSlot);
		if (from && to && *from != AIR) {
			std::swap(*from, *to);
		}
	}
	it->second->craftingResult = lookupRecipe(it->second->craftingSlots);

	enet_packet_destroy(packet);
}

void networkTick(std::unordered_map<ENetPeer *, std::optional<Player>>& clients) {
	static std::unordered_map<ENetPeer *, std::vector<Vec3Int>> chunkRequests;
	static std::vector<BlockUpdatePacket> blockUpdatesVec;
	static std::vector<Item> itemsVec;
	const std::vector<Vec3Int> emptyChunkRequests;

	tickItemPhysics(itemsVec);
	tickItemPickup(itemsVec, clients);

	std::vector<PlayerData> playerDataVec = buildPlayerDataVec(clients);

	for (const auto& [peer, clientP] : clients) {
		if (!clientP) {
			std::cout << "skipping client with id: " << peer->connectID
			          << " because it has no player data" << std::endl;
		}

		std::vector<uint8_t> packetBuffer;
		addVecToPacket(packetBuffer, playerDataVec);

		const auto requestIt = chunkRequests.find(peer);
		const std::vector<Vec3Int>& peerChunkRequests =
		    requestIt != chunkRequests.end() ? requestIt->second : emptyChunkRequests;
		addToPacketForEachPeer(packetBuffer, peerChunkRequests, blockUpdatesVec, itemsVec);

		ENetPacket* packet = enet_packet_create(packetBuffer.data(), packetBuffer.size(),
		                                        ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
	}
	chunkRequests.clear();
	blockUpdatesVec.clear();

	ENetEvent event;
	while (enet_host_service(server, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("A client connected from %x:%u.\n",
				       event.peer->address.host, event.peer->address.port);
				clients[event.peer] = std::nullopt;
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				handleReceivePacket(event.packet, event.peer, clients,
				                    chunkRequests, blockUpdatesVec, itemsVec);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				puts("disconnected from server");
				clients.erase(event.peer);
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
		}
	}
}

void updateServer(std::stop_token st) {
	std::unordered_map<ENetPeer *, std::optional<Player>> clients;
	while (!st.stop_requested()) {
		networkTick(clients);
		//std::this_thread::sleep_for(std::chrono::milliseconds(30));
		WaitTime(0.03);
	}

	// send a disconnect packet to all clients
	for (const auto& [peer, clientP] : clients) {
		enet_peer_disconnect(peer, 0);
	}
	enet_host_flush(server);
	enet_host_destroy(server);
}

bool hostServer(uint16_t port) {
	serverMap.worldGenInit();
	initRecipes();

	address.host = ENET_HOST_ANY;
	address.port = port;

	server = enet_host_create(&address, 32/*maxClients*/, 2/*maxChannels*/, 0/*incomingBandwidth*/, 0/*outgoingBandwidth*/);

	if (server == NULL) {
		fprintf(stderr, "Cant create Enet server \n");
		return false;
	}

	printf("server created successfully\n");
	return true;
}
