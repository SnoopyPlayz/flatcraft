#pragma once
#include <string>
#include <raylib.h>

Texture2D useTexture(const std::string& Path);
void setAndLoadFont(const std::string& Path);
void drawTextSDF(const std::string& text, float posX, float posY, int fontSize, Color color);

extern Font fontSDF; // the current font
