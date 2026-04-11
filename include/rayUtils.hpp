#pragma once
#include <functional>
#include <string>
#include <vector>
#include <raylib.h>
#include <magic_enum.hpp>


struct Texture2DInstance {
	float height = 0;
	std::function<void()> drawCall = nullptr;
};

Texture2D useTexture(const std::string& Path);
void setAndLoadFont(const std::string& Path);
void drawTextSDF(const std::string& text, float posX, float posY, int fontSize, Color color);
void unloadShaders();
Shader useShader(const std::string& Path, const std::string& fragmentPath);
void DrawTextureWithRot(Texture tex, float x, float y, float rot, Color col, float scale = 1);
void queueDraw3D(float height, std::function<void()> drawCall);
std::string stringToLower(std::string s);
void drawTextSDF3D(const std::string& text, float posX, float posY, int fontSize, Color color);

void drawAllTextures3D();
void drawTexture3DInstances(const std::vector<Texture2DInstance>& instances);
Vector2 getWorldRenderOffset();

std::string getEnumName(auto enumVal){
	auto enumName = magic_enum::enum_name(enumVal);
	std::string nameStr { enumName };
	return stringToLower(nameStr);
}

extern Font fontSDF; // the current font
