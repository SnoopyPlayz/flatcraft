#include <cassert>
#include <map>
#include <stdio.h>
#include <raylib.h>
#include <utility>
#include <vector>
#include "rayUtils.hpp"
#include <cstdint>

typedef int32_t int32;
typedef uint8_t uint8;


const int CHUNK_SIZE = 32;

struct Vec3Int {
    int32 x;
    int32 y;
    int32 z;

    bool operator<(const Vec3Int& other) const {
        if (x != other.x)
            return x < other.x;
        
        if (y != other.y)
            return y < other.y;
        
        return z < other.z;
    }
};

struct chunk {
	uint8 t[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};


std::map<Vec3Int, chunk> map;
void createChunk(Vec3Int xyz){
	auto result = map.emplace(std::make_pair(xyz, chunk{}));
	assert(result.second);
}

chunk& findChunk(Vec3Int search_key){
	auto it = map.find(search_key);
	assert(it != map.end());

	return it->second;
}

bool validChunk(Vec3Int pos){
	auto it = map.find(pos);
	if (it != map.end()) {
		return true;
	}
	return false;
}

void setBlock(Vec3Int pos, int block){
	int x = pos.x / CHUNK_SIZE;
	int y = pos.y / CHUNK_SIZE;
	int z = pos.z / CHUNK_SIZE;

	printf("x:%d \n", x);
	printf("y:%d \n", y);
	printf("z:%d \n", z);
	if (!validChunk({x,y,z})) {
		createChunk({x,y,z});
	}
	chunk& c = findChunk({x,y,z});
	c.t[pos.x % CHUNK_SIZE][pos.y % CHUNK_SIZE][pos.z % CHUNK_SIZE] = block;
}

int main(){
	InitWindow(1280, 720, "test");
	SetTargetFPS(60);

	createChunk({0,0,0});
	createChunk({1,0,0});
	createChunk({0,1,0});
	createChunk({0,0,1});
	chunk& c = findChunk({0,0,0});
	c.t[0][0][0] = 1;
	c.t[1][0][1] = 1;

	const int blockSize = 64;
	Camera2D camera = { 0 };
	camera.target = {0,0};
	camera.offset = {0,0};
	camera.zoom = 1;
	camera.rotation = 0;

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(camera);

		const int cameraSpeed = 10;

		if (IsKeyDown(KEY_S))
			camera.target.y += cameraSpeed;
		
		if (IsKeyDown(KEY_A))
			camera.target.x -= cameraSpeed;

		if (IsKeyDown(KEY_D))
			camera.target.x += cameraSpeed;

		if (IsKeyDown(KEY_W))
			camera.target.y -= cameraSpeed;
		

		for (const auto& pair : map) {
			for (int x = 0; x < CHUNK_SIZE; x++) {
				for (int y = 0; y < CHUNK_SIZE; y++) {
					for (int z = 0; z < CHUNK_SIZE; z++) {
						if (pair.second.t[x][y][z] == 1) {
							const int chunk_world_x = pair.first.x * CHUNK_SIZE * blockSize;
							const int chunk_world_z = pair.first.z * CHUNK_SIZE * blockSize;

							// Local Block Position (relative to chunk start)
							const int block_local_x = x * blockSize;
							const int block_local_z = z * blockSize;

							// Final World Position
							const int world_x = chunk_world_x + block_local_x;
							const int world_z = chunk_world_z + block_local_z;
							DrawTexture(useTexture("grass.png"), world_x, world_z, WHITE);
						}
					}
				}
			}
		}

		
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			Vector2 mouseScreen = GetMousePosition();
			Vector2 m = GetScreenToWorld2D(mouseScreen, camera);

			setBlock({(int)m.x / blockSize,0,(int)m.y / blockSize}, 1);
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			Vector2 mouseScreen = GetMousePosition();
			Vector2 m = GetScreenToWorld2D(mouseScreen, camera);

			setBlock({(int)m.x / blockSize,0,(int)m.y / blockSize}, 0);
		}

		EndMode2D();
		EndDrawing();
	}

	return 0;
}
