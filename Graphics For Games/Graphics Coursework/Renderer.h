#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/HeightMap.h"
#include "../../nclgl//OBJMesh.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window & parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	virtual void UpdateScene(float msec);

protected:
	void DrawHeightmap();
	void DrawLava();
	void DrawSkybox();

	Shader * lightShader;
	Shader * reflectShader;
	Shader * skyboxShader;

	HeightMap * heightMap;
	Mesh * quad;
	//OBJMesh* rock;

	Light * light;
	Camera * camera;

	GLuint cubeMap;

	float waterRotate;
};