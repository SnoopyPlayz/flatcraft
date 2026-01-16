#include <raylib.h>
#include <map>
#include <string>
#include <stdio.h>
#include "rayUtils.hpp"

std::map<std::string, Texture2D> textureMap;
Model plane;

void initRayUtils(){
	plane = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1.0f, 1.0f));
}

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


void DrawTexture3D(Vector3 pos, const std::string& Path, Color tint){
	plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = useTexture(Path);
	DrawModel(plane, pos, 1.0f, tint);
}
