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

static auto &vec3Axis(auto &v, int axis) {
	if (axis == 0) {
		return v.x;
	}
	if (axis == 1) {
		return v.y;
	}
	return v.z;
}

float physicsReaction(Vector3 &pos, float &vel, int axis){
	float &posAxis = vec3Axis(pos, axis);

	Vector3 playerTopLeft = pos;
	playerTopLeft.x -= 0.5;
	playerTopLeft.y += 1.0;
	playerTopLeft.z -= 0.5;
	const float playerAxis = vec3Axis(playerTopLeft, axis);

	Vec3Int pTopLeft = toVec3Int(playerTopLeft);
	RADUIS(1){
		Vec3Int blockPos = pTopLeft + (Vec3Int){x,y,z};
		if (getBlock(blockPos) == AIR) {
			continue;
		}
		if (AABBColBox3d(playerTopLeft, {1,1,1}, blockPos.toVec3(), {1,1,1})){
			Vec3Int posx = toVec3Int(pos);
			posx.y += 1;
			const float blockAxis = vec3Axis(blockPos, axis);

			if (getBlock(posx) != AIR) {
				vec3Axis(posx, axis) += 1;
				if (getBlock(posx) == AIR && playerAxis > blockAxis) {
					posAxis += 0.5;
				}
				vec3Axis(posx, axis) -= 2;
				if (getBlock(posx) == AIR && playerAxis < blockAxis) {
					posAxis -= 0.5;
				}
			}

			posAxis -= vel;
			vel = 0.0;
			if (axis == 0) {
				posAxis = std::floor(posAxis) + 0.5;
			} else if (axis == 1) {
				posAxis = std::floor(posAxis);
			} else {
				posAxis = std::floor(posAxis) + 0.5;
			}
		}
	}

	return posAxis;
}
