#include "HeightMap.h"

HeightMap::HeightMap() {

	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH - 1)*(RAW_HEIGHT - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];

	noise = new Perlin(6, 6, 2, 76); //octaves, noise freq, amplitude, seed

	for (int x = 0; x < RAW_WIDTH; ++x) {
		for (int z = 0; z < RAW_HEIGHT; ++z) {
			int offset = (x * RAW_WIDTH) + z;

			float yVal = (noise->Get((float)x/RAW_WIDTH,(float)z/RAW_HEIGHT)) * (RAW_HEIGHT / 2);
			yVal = (float)abs(yVal);
			vertices[offset] = Vector3(
				x * HEIGHTMAP_X, yVal * HEIGHTMAP_Y, z * HEIGHTMAP_Z);

			textureCoords[offset] = Vector2(
				x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}

	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH - 1; ++x) {
		for (int z = 0; z < RAW_HEIGHT - 1; ++z) {
			int a = (x * (RAW_WIDTH)) + z;
			int b = ((x + 1) * (RAW_WIDTH)) + z;
			int c = ((x + 1) * (RAW_WIDTH)) + (z + 1);
			int d = (x * (RAW_WIDTH)) + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;
		}
	}

	GenerateNormals();
	GenerateTangents();

	BufferData();
}

float HeightMap::pollMap(int x, int z) {
	float yVal = (noise->Get((float)x / RAW_WIDTH, (float)z / RAW_HEIGHT)) * (RAW_HEIGHT / 2);
	return yVal;
}
