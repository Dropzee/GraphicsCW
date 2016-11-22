#include "Renderer.h"

Renderer::Renderer(Window & parent) : OGLRenderer(parent) {
	camera = new Camera();
	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");
	quad = Mesh::GenerateQuad();
	emitter = new ParticleEmitter(Vector3(3000,0,3000));

	camera->SetPosition(Vector3(RAW_WIDTH * HEIGHTMAP_X / 2.0f,
		500.0f, RAW_WIDTH * HEIGHTMAP_X));

	light = new Light(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 2.0f), 500.0f,
		(RAW_HEIGHT * HEIGHTMAP_Z / 2.0f)),
		Vector4(0.9f, 0.9f, 1.0f, 1),
		(RAW_WIDTH * HEIGHTMAP_X));

	reflectShader = new Shader(SHADERDIR"PerPixelVertex.glsl",
		SHADERDIR"reflectFragment.glsl");
	skyboxShader = new Shader(SHADERDIR"skyboxVertex.glsl",
		SHADERDIR"skyboxFragment.glsl");
	lightShader = new Shader(SHADERDIR"PerPixelVertex.glsl",
		SHADERDIR"PerPixelFragment.glsl");
	particleShader = new Shader(SHADERDIR"particleVertex.glsl",
		SHADERDIR"particleFragment.glsl",
		SHADERDIR"particleGeometry.glsl");

	if (!reflectShader->LinkProgram() || !lightShader->LinkProgram() ||
		!skyboxShader->LinkProgram() || !particleShader->LinkProgram()) {
		return;
	}

	quad->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"diffus.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	quad->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"normal.tga", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	heightMap->SetTexture(SOIL_load_OGL_texture(
		TEXTUREDIR"lavaland.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	/*heightMap->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));*/

	/*cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"left.jpg", TEXTUREDIR"right.jpg",
		TEXTUREDIR"top.jpg", TEXTUREDIR"bot.jpg",
		TEXTUREDIR"back.jpg", TEXTUREDIR"front.jpg",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);*/

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/box1/violentdays_lf.png", TEXTUREDIR"/box1/violentdays_rt.png",
		TEXTUREDIR"/box1/violentdays_up.png", TEXTUREDIR"/box1/violentdays_dn.png",
		TEXTUREDIR"/box1/violentdays_ft.png", TEXTUREDIR"/box1/violentdays_bk2.png",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);

	/*cubeMap2 = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/box1/violentdays_lf.png", TEXTUREDIR"/box1/violentdays_rt.png",
		TEXTUREDIR"/box1/violentdays_up.png", TEXTUREDIR"/box1/violentdays_dn.png",
		TEXTUREDIR"/box1/violentdays_ft.png", TEXTUREDIR"/box1/violentdays_bk.png",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);*/

	if (!cubeMap || !quad->GetTexture() || !heightMap->GetTexture() /* ||
		!heightMap->GetBumpMap()*/) {
		return;
	}

	//rock = new OBJMesh(MESHDIR"/Stone Pack1_Stone_5/Stone Pack1_Stone_5.obj");
	////OBJMesh* rock = new OBJMesh(MESHDIR"Boulder3.obj");

	//rock->SetTexture(SOIL_load_OGL_texture(
	//	TEXTUREDIR"lavaland.jpg", SOIL_LOAD_AUTO,
	//	SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	
	SetTextureRepeating(quad->GetTexture(), true);
	SetTextureRepeating(heightMap->GetTexture(), true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);
	init = true;

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete light;
	delete emitter;
	currentShader = 0;
}
void Renderer::UpdateScene(float msec) {
	emitter->Update(msec);
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
}


void Renderer::RenderScene() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	
	DrawSkybox();
	DrawHeightmap();
	DrawLava();
	DrawEmitter();

	SwapBuffers();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	SetCurrentShader(skyboxShader);

	UpdateShaderMatrices();
	quad->Draw();

	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {

	SetCurrentShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float *)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();

	glUseProgram(0);
}

void Renderer::DrawLava() {
	SetCurrentShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float *)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	float heightX = (RAW_WIDTH * HEIGHTMAP_X / 2.0f);

	float heightY = 256 * HEIGHTMAP_Y / 3.0f;

	float heightZ = (RAW_HEIGHT * HEIGHTMAP_Z / 2.0f);

	modelMatrix =
		Matrix4::Translation(Vector3(heightX, heightY, heightZ)) *
		Matrix4::Scale(Vector3(heightX, 1, heightZ)) *
		Matrix4::Rotation(90, Vector3(1.0f, 0.0f, 0.0f));

	textureMatrix = Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(/*waterRotate*/ 0 , Vector3(0.0f, 0.0f, 1.0f));
	UpdateShaderMatrices();

	quad->Draw();

	glUseProgram(0);
}

void Renderer::DrawEmitter() {
	//glClearColor(0, 0, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetCurrentShader(particleShader);
	SetShaderLight(*light);

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	SetShaderParticleSize(emitter->GetParticleSize());
	emitter->SetParticleSize(8.0f);
	emitter->SetParticleVariance(1.0f);
	emitter->SetLaunchParticles(16.0f);
	emitter->SetParticleLifetime(2000.0f);
	emitter->SetParticleSpeed(0.1f);
	UpdateShaderMatrices();

	emitter->Draw();

	glUseProgram(0);

}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "particleSize"), f);
}

