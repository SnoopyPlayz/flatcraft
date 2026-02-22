#pragma once
#include <string>
#include <raylib.h>
#include "vector.hpp"

Texture2D useTexture(const std::string& Path);
void setAndLoadFont(const std::string& Path);
void drawTextSDF(const std::string& text, float posX, float posY, int fontSize, Color color);
void drawTexture3D(Texture2D texture, Vector3 vec, Color tint);
void drawRect3D(Vector3 pos, Color tint);
void unloadShaders();
Shader useShader(const std::string& Path);
void drawTexture3DRot(Texture2D texture, Vector3 pos, Color tint, float rotation);

void drawAllTextures3D();


extern Font fontSDF; // the current font
