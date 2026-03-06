#include <algorithm>
#include <optional>
#include <raylib.h>
#include <map>
#include <string>
#include <stdio.h>
#include <vector>
#include "rayUtils.hpp"
#include <map.hpp>

std::string absolutePath = "res/"; // absolutePath + fullPath + givenPath
std::map<std::string, Texture2D> textureMap;
std::map<std::string, Shader> shaderMap;
Font fontSDF;

Texture2D useTexture(const std::string& Path){
	std::string fullPath = absolutePath + Path;

	if (auto search = textureMap.find(fullPath); search != textureMap.end()){
		return search->second;
	}else {
		Texture2D tex = LoadTexture(fullPath.c_str());
		textureMap.insert({fullPath, tex});
		return tex;
	}
}

std::string stringToLower(std::string s){
	for (auto & c: s){
		c = std::tolower((unsigned char)c);
	}
	return s;
}

void unloadShaders(){
	for (const auto& pair : shaderMap) {
		UnloadShader(pair.second);
	}
}

Shader useShader(const std::string& Path){
	std::string fullPath = absolutePath + Path;

	if (auto search = shaderMap.find(fullPath); search != shaderMap.end()){
		return search->second;
	}else {
		Shader shader = LoadShader(0, fullPath.c_str());
		shaderMap.insert({fullPath, shader});
		return shader;
	}
}

void setAndLoadFont(const std::string& Path){
	std::string fullPath = absolutePath + "font/" + Path;

	int fileSize = 0;
	const int fontSize = 73;

	unsigned char *fileData = LoadFileData(fullPath.c_str(), &fileSize);

	// Define the Polish characters (Unicode IDs)
	int polishCodepoints[] = {
		0x0104, 0x0105, 0x0106, 0x0107, 0x0118, 0x0119, // ą, Ą, ć, Ć, ę, Ę
		0x0141, 0x0142, 0x0143, 0x0144, 0x00D3, 0x00F3, // ł, Ł, ń, Ń, ó, Ó
		0x015A, 0x015B, 0x0179, 0x017A, 0x017B, 0x017C  // ś, Ś, ź, Ź, ż, Ż
	};

	// Combine them with ASCII 32-126
	int allCodepoints[256];
	int count = 0;
	// Add ASCII
	for (int i = 32; i <= 126; i++) allCodepoints[count++] = i;

	// Add Polish
	for (int i = 0; i < 18; i++) allCodepoints[count++] = polishCodepoints[i];

	// SDF font gen
	fontSDF.baseSize = fontSize;
	fontSDF.glyphCount = count;
	fontSDF.glyphs = LoadFontData(fileData, fileSize, fontSize, allCodepoints, count, FONT_SDF);
	// pack px: 4 pack method: 1 (skyline algorithm)
	Image atlas = GenImageFontAtlas(fontSDF.glyphs, &fontSDF.recs, count, fontSize, 4, 1);
	fontSDF.texture = LoadTextureFromImage(atlas);
	UnloadImage(atlas);
	UnloadFileData(fileData);

	SetTextureFilter(fontSDF.texture, TEXTURE_FILTER_BILINEAR);
}

void drawTextSDF(const std::string& text, float posX, float posY, int fontSize, Color color){
	BeginShaderMode(useShader("shaders/sdf.fs"));
	DrawTextEx(fontSDF, text.c_str(), {posX, posY}, fontSize, 0, color);
	EndShaderMode();
}

struct Texture2DInstance {
	Vector3 position;
	std::optional<Texture2D> texture;
	Color tint;
	float rotation;
};

std::vector<Texture2DInstance> vertices;

void DrawTextureWithRot(Texture tex, float x, float y, float rot, Color col){
	float texX = tex.width;
	float texY = tex.height;

	struct Rectangle posAndSize= (Rectangle){x + texX / 2, y + texY / 2, texX, texY};
	struct Rectangle texSize = (Rectangle){0, 0, texX, texY};

	DrawTexturePro(tex, texSize, posAndSize, (Vector2){(float)texX / 2, (float)texY / 2}, rot, col);
}

void drawTexture3DRot(Texture2D texture, Vector3 pos, Color tint, float rotation){
	vertices.push_back({{pos.x, pos.y, pos.z}, texture, tint, rotation});
}

void drawTexture3D(Texture2D texture, Vector3 pos, Color tint){
	drawTexture3DRot(texture, pos, tint, 0);
}

void drawRect3D(Vector3 pos, Color tint){
	vertices.push_back({{pos.x, pos.y, pos.z}, std::nullopt, tint, 0});
}

void drawAllTextures3D(){
	std::sort(vertices.begin(), vertices.end(), [](const auto& a, const auto& b) {
		return (a.position.y < b.position.y);
	});

	for (const Texture2DInstance& tex: vertices) {
		if (tex.texture.has_value()) {
			DrawTextureWithRot(tex.texture.value(), tex.position.x, tex.position.z, tex.rotation, tex.tint);
			//DrawTextureEx(tex.texture.value(), {tex.position.x, tex.position.z}, tex.rotation, 1, tex.tint);
		}else{
			DrawRectangle(tex.position.x, tex.position.z, BLOCK_SIZE, BLOCK_SIZE, tex.tint);
		}
	}
	vertices.clear();
}
