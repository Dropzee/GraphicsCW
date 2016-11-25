#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include "Mesh.h"
#include "Perlin.h"

#define RAW_WIDTH 1000
#define RAW_HEIGHT 1000

#define HEIGHTMAP_X 16.0f
#define HEIGHTMAP_Z 16.0f
#define HEIGHTMAP_Y 1.25f
#define HEIGHTMAP_TEX_X 1.0f / 16.0f
#define HEIGHTMAP_TEX_Z 1.0f / 16.0f

class HeightMap : public Mesh {
public:
	HeightMap(std::string name);
	~HeightMap(void) {};

	float pollMap(int x, int z);

protected:
	Perlin * noise;
};