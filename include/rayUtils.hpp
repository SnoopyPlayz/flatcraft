#pragma once
#include <optional>
#include <string>
#include <vector>
#include <raylib.h>
#include <magic_enum.hpp>


struct Texture2DInstance {
	Vector3 position;
	std::optional<Texture2D> texture;
	Color tint;
	float rotation;
};

Texture2D useTexture(const std::string& Path);
void setAndLoadFont(const std::string& Path);
void drawTextSDF(const std::string& text, float posX, float posY, int fontSize, Color color);
void drawTexture3D(Texture2D texture, Vector3 vec, Color tint);
void drawRect3D(Vector3 pos, Color tint);
void unloadShaders();
Shader useShader(const std::string& Path);
void drawTexture3DRot(Texture2D texture, Vector3 pos, Color tint, float rotation);
std::string stringToLower(std::string s);


void drawAllTextures3D();
void drawTexture3DInstances(const std::vector<Texture2DInstance>& instances);

std::string getEnumName(auto enumVal){
	auto enumName = magic_enum::enum_name(enumVal);
	std::string nameStr { enumName };
	return stringToLower(nameStr);
}

extern Font fontSDF; // the current font
