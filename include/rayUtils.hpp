#pragma once
#include <string>
#include <raylib.h>

Texture2D useTexture(const std::string& Path);
void DrawTexture3D(Vector3 pos, const std::string& Path, Color tint);
void initRayUtils();
