#include "Renderer.h"

Renderer::Renderer(Window & parent) : OGLRenderer(parent) {
	camera = new Camera();
	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");
	quad = Mesh::GenerateQuad();
	flash = Mesh::GenerateQuad();
	emitterBubble = new ParticleEmitter(Vector3(1000,110,1000), BUBBLE);
	emitterExplode = new ParticleEmitter(Vector3(0, 0, 0), EXPLOSION);
	emitterSteam = new ParticleEmitter(Vector3(1000, 110, 1000), STEAM);

	camera->SetPosition(Vector3(RAW_WIDTH * HEIGHTMAP_X / 2.0f,
		500.0f, RAW_WIDTH * HEIGHTMAP_X));

	light = new Light(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 2.0f), 500.0f,
		(RAW_HEIGHT * HEIGHTMAP_Z / 2.0f)),
		Vector4(0.9f, 0.9f, 1.0f, 1),
		(RAW_WIDTH * HEIGHTMAP_X));

	basicShader = new Shader(SHADERDIR"basicVertex.glsl",
		SHADERDIR"colourFragment.glsl");
	reflectShader = new Shader(SHADERDIR"PerPixelVertex.glsl",
		SHADERDIR"reflectFragment.glsl");
	skyboxShader = new Shader(SHADERDIR"skyboxVertex.glsl",
		SHADERDIR"skyboxFragment.glsl");
	lightShader = new Shader(SHADERDIR"PerPixelVertex.glsl",
		SHADERDIR"PerPixelFragment.glsl");
	particleShader = new Shader(SHADERDIR"particleVertex.glsl",
		SHADERDIR"particleFragment.glsl",
		SHADERDIR"particleGeometry.glsl");
	texShader = new Shader(SHADERDIR"TexturedVertex.glsl", 
		SHADERDIR"TexturedFragment.glsl");

	if (!reflectShader->LinkProgram() || !lightShader->LinkProgram() ||
		!skyboxShader->LinkProgram() || !particleShader->LinkProgram() ||
		!texShader->LinkProgram() || !basicShader->LinkProgram()) {
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

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/box1/violentdays_lf.png", TEXTUREDIR"/box1/violentdays_rt.png",
		TEXTUREDIR"/box1/violentdays_up.png", TEXTUREDIR"/box1/violentdays_dn.png",
		TEXTUREDIR"/box1/violentdays_ft.png", TEXTUREDIR"/box1/violentdays_bk.png",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);	
	
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);
	prof = false;

	if (!cubeMap || !quad->GetTexture() || !heightMap->GetTexture()/* ||
		!heightMap->GetBumpMap()*/) {
		return;
	}

	//rock = new OBJMesh(MESHDIR"/Stone Pack1_Stone_5/Stone Pack1_Stone_5.obj");
	////OBJMesh* rock = new OBJMesh(MESHDIR"Boulder3.obj");

	//rock->SetTexture(SOIL_load_OGL_texture(
	//	TEXTUREDIR"lavaland.jpg", SOIL_LOAD_AUTO,
	//	SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	
	waterRotate = 0;

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

	explosion = false;
	explodeCount = 0;
}

Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete texShader;
	delete particleShader;
	delete basicFont;
	delete light;
	delete emitterBubble;
	delete emitterExplode;
	delete emitterSteam;
	currentShader = 0;
}
void Renderer::UpdateScene(float msec) {
	emitterBubble->Update(msec);
	emitterExplode->Update(msec);
	emitterSteam->Update(msec);
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += msec / 1000;

	fps = 1000 / msec;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	memory = pmc.WorkingSetSize;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P)) {
		prof = !prof;
	}

	if(Window::GetKeyboard()->KeyTriggered(KEYBOARD_X) && !explosion) {
		explosion = true;
	}
}


void Renderer::RenderScene() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	
	DrawSkybox();
	DrawHeightmap();
	DrawLava();
	DrawEmitter();

	if (explosion) {
		Explode();
	}

	if (prof) {
		Profile();
	}

	modelMatrix.ToIdentity();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	UpdateShaderMatrices();

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
		Matrix4::Translation(Vector3(0.0f, waterRotate*0.01, 1.0f));
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

	SetShaderParticleSize(emitterBubble->GetParticleSize());
	emitterBubble->SetParticleSize(10.0f);
	emitterBubble->SetParticleVariance(1.0f);
	emitterBubble->SetLaunchParticles(100.0f);
	emitterBubble->SetParticleLifetime(2000.0f);
	emitterBubble->SetParticleSpeed(0.05f);
	UpdateShaderMatrices();

	emitterBubble->Draw();

	glUseProgram(0);

}

void Renderer::Profile() {

	SetCurrentShader(texShader);

												
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	DrawText("FPS: " + to_string(fps), Vector3(0, 0, 0), 16.0f);
	DrawText("Memory Usage:" + to_string(memory / 1048576.0f) + "Mb", Vector3(0, 16, 0), 16.0f);

	//DrawText("This is perspective text!!!!", Vector3(0, 0, -1000), 64.0f, true); //perspective

	glUseProgram(0);

}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "particleSize"), f);
}

void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective) {
	//Create a new temporary TextMesh, using our line of text and our font
	TextMesh* mesh = new TextMesh(text, *basicFont);

	//This just does simple matrix setup to render in either perspective or
	//orthographic mode, there's nothing here that's particularly tricky.
	if (perspective) {
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	}
	else {
		//In ortho mode, we subtract the y from the height, so that a height of 0
		//is at the top left of the screen, which is more intuitive
		//(for me anyway...)
		modelMatrix = Matrix4::Translation(Vector3(position.x, height - position.y, position.z)) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);
	}
	//Either way, we update the matrices, and draw the mesh
	UpdateShaderMatrices();
	mesh->Draw();

	delete mesh; //Once it's drawn, we don't need it anymore!
}

void Renderer::Explode() {

	explodeCount++;

	if (explodeCount < 100) {
		SetCurrentShader(particleShader);
		SetShaderLight(*light);

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

		modelMatrix = Matrix4::Translation(Vector3(400, 250, 0)) * Matrix4::Scale(Vector3(1, 1, 1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);

		UpdateShaderMatrices();

		emitterExplode->Draw();
	}
	else if (explodeCount < 150) {
		//fade in
		SetCurrentShader(basicShader);
		flash->Draw();
	}
	else if (explodeCount == 150) {
		cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/box1/violentdays_lf.png", TEXTUREDIR"/box1/violentdays_rt.png",
		TEXTUREDIR"/box1/violentdays_up.png", TEXTUREDIR"/box1/violentdays_dn.png",
		TEXTUREDIR"/box1/violentdays_ft.png", TEXTUREDIR"/box1/violentdays_bk2.png",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);
	}
	else if (explodeCount < 200) {
		//fade out
	}
	else {
		explosion = false;
		explodeCount = 0;
	}

	glUseProgram(0);
}