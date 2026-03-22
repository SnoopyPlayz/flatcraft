#pragma once
#include <raylib.h>
#include <raymath.h>

bool AABBColBox3d(Vector3 pos, Vector3 size, Vector3 otherPos, Vector3 otherSize);
bool AABBColBox2d(int x, int y, int size, int xb, int yb, int sizeb);
float physicsReaction(Vector3 &pos, float &vel, int axis);
