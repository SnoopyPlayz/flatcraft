#include <raylib.h>
#include <map>
#include <string>
#include <stdio.h>
#include "rayUtils.hpp"

std::map<std::string, Texture2D> textureMap;

Texture2D useTexture(std::string Path){
	Path = "res/" + Path;

	if (auto search = textureMap.find(Path); search != textureMap.end()){
		return search->second;
	}else {
		Texture2D tex = LoadTexture(Path.c_str());
		textureMap.insert({Path, tex});
		return tex;
	}
}
