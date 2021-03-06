#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/HeightMap.h"
#include "../../nclgl//OBJMesh.h"
#include "ParticleEmitter.h"
#include "TextMesh.h"
#include <string>
#include "psapi.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window & parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	virtual void UpdateScene(float msec);

	void	DrawText(const std::string &text, const Vector3 &position, const float size = 10.0f, const bool perspective = false);

protected:
	void DrawHeightmap();
	void DrawLava();
	void DrawSkybox();
	void DrawEmitter();
	void Profile();
	void Explode();

	Shader * basicShader;
	Shader * lightShader;
	Shader * lavaShader;
	Shader * skyboxShader;
	Shader * particleShader;
	Shader * texShader;

	HeightMap * heightMap;
	Mesh * lava;
	OBJMesh* rock;

	float waterRotate;

	Light * light;
	Camera * camera;

	GLuint cubeMap;
	GLuint cubeMap2;

	void	SetShaderParticleSize(float f);

	ParticleEmitter*	emitterBubble;
	ParticleEmitter*	emitterExplode;
	ParticleEmitter*	emitterSteam[100];
	int GEYSERS = 5;
	bool move[5];
	bool emit[5];

	bool explosion;
	int explodeCount;

	bool prof;
	Font*	basicFont;
	float fps;
	PROCESS_MEMORY_COUNTERS pmc;
	SIZE_T memory;
};
