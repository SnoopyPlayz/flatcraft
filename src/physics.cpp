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

