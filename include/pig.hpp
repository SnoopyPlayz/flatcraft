#pragma once
#include <cstdint>
#include <vector>
#include <raylib.h>

struct Pig {
	uint32_t id;
	Vector3 pos;
	Vector3 velocity;
	float rotation = 0.0f;
};

extern std::vector<Pig> pigs;
void drawPigs();
