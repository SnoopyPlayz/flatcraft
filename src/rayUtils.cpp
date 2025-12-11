#include <raylib.h>
#include <map>
#include <string>
#include <stdio.h>
using namespace std;

map<string, Texture2D> textureMap;

Texture2D useTexture(string Path){
	Path = "res/" + Path;

	if (auto search = textureMap.find(Path); search != textureMap.end()){
		return search->second;
	}else {
		Texture2D tex = LoadTexture(Path.c_str());
		textureMap.insert({Path, tex});
		return tex;
	}
}
