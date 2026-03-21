#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include "camera.hpp"
#include "rayUtils.hpp"
#include "map.hpp"
#include "debug.hpp"
#include "player.hpp"
#include "network.hpp"
#include "vector.hpp"
#include <iostream>

bool AABBColBox2d(int x, int y, int size, int xb, int yb, int sizeb){
	return (x < xb + sizeb && x + size > xb && y < yb + sizeb && y + size > yb);
}

bool AABBColBox3d(Vector3 pos, Vector3 size, Vector3 otherPos, Vector3 otherSize){
	return (
		pos.x < otherPos.x + otherSize.x &&
		pos.x + size.x > otherPos.x &&
		pos.y < otherPos.y + otherSize.y &&
		pos.y + size.y > otherPos.y &&
		pos.z < otherPos.z + otherSize.z &&
		pos.z + size.z > otherPos.z
	);
}

float physicsReaction(Vector3 pos, float vel, float axis, bool &setpos){
	Vector3 playerTopLeft = pos;
	playerTopLeft.x -= 0.5;
	playerTopLeft.y += 1.0;
	playerTopLeft.z -= 0.5;

	Vec3Int pTopLeft = toVec3Int(playerTopLeft);
	RADUIS(2){
		Vec3Int blockPos = pTopLeft + (Vec3Int){x,y,z};
		if (getBlock(blockPos) == AIR) {
			continue;
		}
		if (AABBColBox3d(playerTopLeft, {1,1,1}, blockPos.toVec3(), {1,1,1})){
			pos.x -= vel;
			axis -= vel;
			Vec3Int posx = toVec3Int(pos);
			posx.y += 1;

			if (getBlock(posx) != AIR) {
				posx.x += 1;
				if (getBlock(posx) == AIR) {
					if (playerTopLeft.x > blockPos.toVec3().x) {
						pos.x += 0.5;
						axis += 0.5;
					}
				}
				posx.x -= 2;
				if (getBlock(posx) == AIR) {
					if (playerTopLeft.x < blockPos.toVec3().x){
						pos.x -= 0.5;
						axis -= 0.5;
					}
				}
			}
			pos.x = std::floor(pos.x) + 0.5;
			axis = std::floor(pos.x) + 0.5;
			setpos = true;
		}
	}
	return axis;
}
