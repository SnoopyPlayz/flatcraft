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

constexpr float GRAVITY_ACCELERATION = 0.015f;
constexpr float MAX_FALL_SPEED = 0.6f;

bool AABBcolBox(int x, int y, int size, int xb, int yb, int sizeb){
	return (x < xb + sizeb && x + size > xb && y < yb + sizeb && y + size > yb);
}

bool AABBcolBox(Vector3 pos, Vector3 size, Vector3 otherPos, Vector3 otherSize){
	return (
		pos.x < otherPos.x + otherSize.x &&
		pos.x + size.x > otherPos.x &&
		pos.y < otherPos.y + otherSize.y &&
		pos.y + size.y > otherPos.y &&
		pos.z < otherPos.z + otherSize.z &&
		pos.z + size.z > otherPos.z
	);
}

Vector3 AABBcollisionResponse(Vector3 center, Vector3 size, Vector3 otherCenter, Vector3 otherSize){
	const Vector3 halfSize = size * 0.5f;
	const Vector3 otherHalfSize = otherSize * 0.5f;
	const Vector3 pos = center - halfSize;
	const Vector3 otherPos = otherCenter - otherHalfSize;

	if (!AABBcolBox(pos, size, otherPos, otherSize)) {
		return {0.0f, 0.0f, 0.0f};
	}

	const Vector3 overlap1 = (otherPos + otherSize) - pos;
	const Vector3 overlap2 = (pos + size) - otherPos;
	const Vector3 penetration = Vector3Min(overlap1, overlap2);

	if (penetration.x <= penetration.y && penetration.x <= penetration.z) {
		return (overlap1.x < overlap2.x)
			? Vector3{-penetration.x, 0.0f, 0.0f}
			: Vector3{penetration.x, 0.0f, 0.0f};
	}

	if (penetration.y <= penetration.x && penetration.y <= penetration.z) {
		return (overlap1.y < overlap2.y)
			? Vector3{0.0f, -penetration.y, 0.0f}
			: Vector3{0.0f, penetration.y, 0.0f};
	}

	return (overlap1.z < overlap2.z)
		? Vector3{0.0f, 0.0f, -penetration.z}
		: Vector3{0.0f, 0.0f, penetration.z};
}

float AABBcollisionResponseAxis(Vector3 center, Vector3 size, Vector3 otherCenter, Vector3 otherSize, int axis){
	if (axis == 1) {
		const Vector3 halfSize = size * 0.5f;
		const Vector3 otherHalfSize = otherSize * 0.5f;
		const Vector3 pos = center - halfSize;
		const Vector3 otherPos = otherCenter - otherHalfSize;

		if (!AABBcolBox(pos, size, otherPos, otherSize)) {
			return 0.0f;
		}

		const Vector3 overlap1 = (otherPos + otherSize) - pos;
		const Vector3 overlap2 = (pos + size) - otherPos;
		const float penetration = fminf(overlap1.y, overlap2.y);
		return (overlap1.y < overlap2.y) ? -penetration : penetration;
	}

	const Vector3 response = AABBcollisionResponse(center, size, otherCenter, otherSize);
	if (axis == 0) return response.x;
	if (axis == 2) return response.z;
	return 0.0f;
}

float applyGravity(float currentYVelocity){
	const float nextVelocity = currentYVelocity - GRAVITY_ACCELERATION;
	return fmaxf(nextVelocity, -MAX_FALL_SPEED);
}
