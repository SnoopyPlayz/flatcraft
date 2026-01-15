#include <raylib.h>
#include <map>
#include <string>
#include <stdio.h>
#include "rayUtils.hpp"

std::map<std::string, Texture2D> textureMap;

Texture2D useTexture(const std::string& Path){
	std::string fullPath = "res/" + Path;

	if (auto search = textureMap.find(fullPath); search != textureMap.end()){
		return search->second;
	}else {
		Texture2D tex = LoadTexture(fullPath.c_str());
		textureMap.insert({fullPath, tex});
		return tex;
	}
}
