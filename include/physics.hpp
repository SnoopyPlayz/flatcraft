#pragma once

#include <raylib.h>

bool AABBcolBox(int x, int y, int size, int xb, int yb, int sizeb);
bool AABBcolBox(Vector3 pos, Vector3 size, Vector3 otherPos, Vector3 otherSize);
Vector3 AABBcollisionResponse(Vector3 center, Vector3 size, Vector3 otherCenter, Vector3 otherSize);
float AABBcollisionResponseAxis(Vector3 center, Vector3 size, Vector3 otherCenter, Vector3 otherSize, int axis);
float applyGravity(float currentYVelocity);
constexpr float JUMP_VELOCITY = 0.22f;
