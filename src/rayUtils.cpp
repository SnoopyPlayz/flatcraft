#include <iostream>
#include <raylib.h>
#include <map>
#include <string>
#include <stdio.h>
#include "rayUtils.hpp"

std::string absolutePath = "res/"; // absolutePath + fullPath + givenPath
std::map<std::string, Texture2D> textureMap;
Font fontSDF;
Shader currentShader;

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

	currentShader = LoadShader(0, (absolutePath + "shaders/sdf.fs").c_str());
	SetTextureFilter(fontSDF.texture, TEXTURE_FILTER_BILINEAR);

}

void drawTextSDF(const std::string& text, float posX, float posY, int fontSize, Color color){
	BeginShaderMode(currentShader);
	DrawTextEx(fontSDF, text.c_str(), {posX, posY}, fontSize, 0, color);
	EndShaderMode();
}
