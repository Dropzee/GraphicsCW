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
	Shader * reflectShader;
	Shader * skyboxShader;
	Shader * particleShader;
	Shader * texShader;

	HeightMap * heightMap;
	Mesh * quad;
	Mesh * flash;
	//OBJMesh* rock;

	float waterRotate;

	Light * light;
	Camera * camera;

	GLuint cubeMap;
	GLuint cubeMap2;

	void	SetShaderParticleSize(float f);

	ParticleEmitter*	emitterBubble;
	ParticleEmitter*	emitterExplode;
	ParticleEmitter*	emitterSteam;

	bool explosion;
	int explodeCount;

	bool prof;
	Font*	basicFont;
	float fps;
	PROCESS_MEMORY_COUNTERS pmc;
	SIZE_T memory;
};
