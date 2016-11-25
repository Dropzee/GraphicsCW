#include "Renderer.h"

//Constructor
Renderer::Renderer(Window & parent) : OGLRenderer(parent) {
	camera = new Camera();
	heightMap = new HeightMap();
	lava = Mesh::GenerateQuad();
	
	//Particle Emitters
	emitterBubble = new ParticleEmitter(Vector3(9000,110,9000), BUBBLE);
	emitterExplode = new ParticleEmitter(Vector3(0, 0, 0), EXPLOSION);
	for (int i = 0; i < GEYSERS; i++) {
		emitterSteam[i] = new ParticleEmitter(Vector3(9000, 110, 9000), STEAM);
	}

	camera->SetPosition(Vector3(RAW_WIDTH * HEIGHTMAP_X / 2.0f,
		500.0f, RAW_WIDTH * HEIGHTMAP_X));

	light = new Light(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 2.0f), 500.0f,
		(RAW_HEIGHT * HEIGHTMAP_Z / 2.0f)),
		Vector4(0.9f, 0.9f, 1.0f, 1),
		(RAW_WIDTH * HEIGHTMAP_X * 10));

	//Shaders
	basicShader = new Shader(SHADERDIR"basicVertex.glsl",
		SHADERDIR"colourFragment.glsl");
	lavaShader = new Shader(SHADERDIR"PerPixelVertex.glsl",
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

	if (!lavaShader->LinkProgram() || !lightShader->LinkProgram() ||
		!skyboxShader->LinkProgram() || !particleShader->LinkProgram() ||
		!texShader->LinkProgram() || !basicShader->LinkProgram()) {
		return;
	}


	//Textures
	lava->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"diffus.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	lava->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"normal.tga", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	heightMap->SetTexture(SOIL_load_OGL_texture(
		TEXTUREDIR"lavaland.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/box1/violentdays_lf.png", TEXTUREDIR"/box1/violentdays_rt.png",
		TEXTUREDIR"/box1/violentdays_up.png", TEXTUREDIR"/box1/violentdays_dn.png",
		TEXTUREDIR"/box1/violentdays_ft.png", TEXTUREDIR"/box1/violentdays_bk.png",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);	
	
	cubeMap2 = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/box1/violentdays_lf.png", TEXTUREDIR"/box1/violentdays_rt.png",
		TEXTUREDIR"/box1/violentdays_up.png", TEXTUREDIR"/box1/violentdays_dn.png",
		TEXTUREDIR"/box1/violentdays_ft.png", TEXTUREDIR"/box1/violentdays_bk2.png",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);

	
	if (!cubeMap || !lava->GetTexture() || 
		!heightMap->GetTexture() || !lava->GetBumpMap()) {
		return;
	}

	SetTextureRepeating(lava->GetTexture(), true);
	SetTextureRepeating(lava->GetBumpMap(), true);
	SetTextureRepeating(heightMap->GetTexture(), true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);


	//Profile Text
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);
	prof = false;

	init = true;

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	//Flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	//Animation Vars
	explosion = false;
	explodeCount = 0;
	waterRotate = 0;
	for (int i = 0; i < GEYSERS; i++) {
		move[i] = false;
		emit[i] = true;
	}

	
}

//Destructor
Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete lava;
	delete lavaShader;
	delete skyboxShader;
	delete lightShader;
	delete texShader;
	delete particleShader;
	delete basicFont;
	delete light;
	delete emitterBubble;
	delete emitterExplode;
	for (int i = 0; i < GEYSERS; i++) {
		delete emitterSteam[i];
	}
	currentShader = 0;
}

//Update
void Renderer::UpdateScene(float msec) {

	//Particle Emitters
	emitterBubble->Update(msec);
	emitterExplode->Update(msec);
	for (int i = 0; i < GEYSERS; i++) {
		emitterSteam[i]->Update(msec);
	}

	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += msec / 1000;

	//Profile Info
	fps = 1000 / msec;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	memory = pmc.WorkingSetSize;

	//Display Profile Toggle
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P)) {
		prof = !prof;
	}

	//Explosion Key
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_X) && !explosion) {
		explosion = true;
	}

	//Camera Shake when Explosion
	if (explosion) {
		Vector3 pos = camera->GetPosition();
		camera->SetPosition(Vector3(pos.x + (rand() % 55) - 25, pos.y + (rand() % 55) - 25, pos.z + (rand() % 55) - 25));
	}

	//Geyser Animation Settings
	for (int i = 0; i < GEYSERS; i++) {

		//Emit in random length bursts
		int num = rand() % 250;
		if (num <= 0) {
			emit[i] = !emit[i];
		}

		//Turn on
		if (emit[i]) {

			//Move to new location
			if (move[i]) {
				int x, z;
				float y = 0;

				//Poll height map for position above lava level
				while (y < 110) {
					x = 9000 + (rand() % 9000) - 4500;
					z = 9000 + (rand() % 9000) - 4500;
					y = heightMap->pollMap(x, z);
				}
				emitterSteam[i]->updatePosition(Vector3(x, y, z));
			}
			emitterSteam[i]->SetLaunchParticles(50);
			move[i] = false;
		}

		//Turn off
		else {
			emitterSteam[i]->SetLaunchParticles(0);
			move[i] = true;
		}
	}
}

//Render
void Renderer::RenderScene() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//Draw Calls
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

	//Reset matricies
	modelMatrix.ToIdentity();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	UpdateShaderMatrices();

	SwapBuffers();
}

//Render Sky Box with alpha blending between 2 textures
void Renderer::DrawSkybox() {

	glDepthMask(GL_FALSE);
	SetCurrentShader(skyboxShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"cubeTex"), 2);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"cubeTex2"), 3);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"blendFactor"), (float)explodeCount/ 100.0f);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap2);

	UpdateShaderMatrices();
	lava->Draw();

	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

//Render HeightMap using Perlin noise
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

//Render Lava Quad with Bump Map
void Renderer::DrawLava() {
	SetCurrentShader(lavaShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float *)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"cubeTex"), 2);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 3);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	float heightX = (RAW_WIDTH * HEIGHTMAP_X / 2.0f);

	float heightY = 256 * HEIGHTMAP_Y / 3.0f;

	float heightZ = (RAW_HEIGHT * HEIGHTMAP_Z / 2.0f);

	modelMatrix =
		Matrix4::Translation(Vector3(heightX, heightY, heightZ)) *
		Matrix4::Scale(Vector3(heightX * 10, 1, heightZ * 10)) *
		Matrix4::Rotation(90, Vector3(1.0f, 0.0f, 0.0f));

	textureMatrix = Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f)) *
		Matrix4::Translation(Vector3(0.0f, waterRotate*0.001, 1.0f));
	UpdateShaderMatrices();

	lava->Draw();

	glUseProgram(0);
}

//Render Particle Emitters (Lava Bubbles & Geysers)
void Renderer::DrawEmitter() {

	SetCurrentShader(particleShader);
	SetShaderLight(*light);

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	//Lava Bubbles
	SetShaderParticleSize(emitterBubble->GetParticleSize());
	emitterBubble->SetParticleSize(10.0f);
	emitterBubble->SetParticleVariance(1.0f);
	emitterBubble->SetLaunchParticles(200.0f);
	emitterBubble->SetParticleLifetime(2000.0f);
	emitterBubble->SetParticleSpeed(0.1f);
	UpdateShaderMatrices();

	emitterBubble->Draw();

	//Geysers
	for (int i = 0; i < GEYSERS; i++) {
		SetShaderParticleSize(emitterSteam[i]->GetParticleSize());
		emitterSteam[i]->SetParticleSize(10.0f);
		emitterSteam[i]->SetParticleVariance(1.0f);
		emitterSteam[i]->SetParticleLifetime(3000.0f);
		emitterSteam[i]->SetParticleSpeed(0.1f);
		UpdateShaderMatrices();

		emitterSteam[i]->Draw();

	}

	glUseProgram(0);

}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "particleSize"), f);
}

//Render Profile Info
void Renderer::Profile() {

	SetCurrentShader(texShader);
								
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	//Draw Text
	DrawText("FPS: " + to_string(fps), Vector3(0, 0, 0), 16.0f);
	DrawText("Memory Usage:" + to_string(memory / 1048576.0f) + "Mb", Vector3(0, 16, 0), 16.0f);

	glUseProgram(0);

}

//Draw Text from extra tutorials
void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective) {
	TextMesh* mesh = new TextMesh(text, *basicFont);

	if (perspective) {
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	}
	else {
		modelMatrix = Matrix4::Translation(Vector3(position.x, height - position.y, position.z)) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);
	}
	UpdateShaderMatrices();
	mesh->Draw();

	delete mesh;
}

//Render Explosion !
void Renderer::Explode() {

	//Animation counter
	explodeCount++;

	//Display loads of flashes/particles
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
	else {
		explosion = false;
	}

	glUseProgram(0);
}