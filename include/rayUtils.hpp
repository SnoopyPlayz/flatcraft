#pragma once
#include <string>
#include <raylib.h>
#include "vector.hpp"

Texture2D useTexture(const std::string& Path);
void setAndLoadFont(const std::string& Path);
void drawTextSDF(const std::string& text, float posX, float posY, int fontSize, Color color);
void drawTexture3D(Texture2D texture, Vector3 vec, Color tint);

void drawAllTextures3D();


extern Font fontSDF; // the current font
