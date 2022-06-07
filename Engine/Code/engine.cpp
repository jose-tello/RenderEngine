//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "assimp_model_loading.h"
#include "Environment.h"

#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
	GLchar  infoLogBuffer[1024] = {};
	GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
	GLsizei infoLogSize;
	GLint   success;

	char versionString[] = "#version 430\n";
	char shaderNameDefine[128];
	sprintf(shaderNameDefine, "#define %s\n", shaderName);
	char vertexShaderDefine[] = "#define VERTEX\n";
	char fragmentShaderDefine[] = "#define FRAGMENT\n";

	const GLchar* vertexShaderSource[] = {
		versionString,
		shaderNameDefine,
		vertexShaderDefine,
		programSource.str
	};
	const GLint vertexShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(vertexShaderDefine),
		(GLint)programSource.len
	};
	const GLchar* fragmentShaderSource[] = {
		versionString,
		shaderNameDefine,
		fragmentShaderDefine,
		programSource.str
	};
	const GLint fragmentShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(fragmentShaderDefine),
		(GLint)programSource.len
	};

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, vshader);
	glAttachShader(programHandle, fshader);
	glLinkProgram(programHandle);
	glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	glUseProgram(0);

	glDetachShader(programHandle, vshader);
	glDetachShader(programHandle, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return programHandle;
}


u32 CreateProgram(App* app, const char* filepath, const char* programName)
{
	u32 ret = LoadProgram(app, filepath, programName);
	Program& program = app->programs[ret];

	int attributeCount;
	glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

	for (int i = 0; i < attributeCount; ++i)
	{
		char* name = new char[1000];
		int length;
		int size;
		GLenum type;

		glGetActiveAttrib(program.handle, i, 1000, &length, &size, &type, name);
		u8 attributeLocation = glGetAttribLocation(program.handle, name);

		u8 attributeCount = 0;
		switch (type)
		{
		case GL_FLOAT:
			attributeCount = 1;
			break;

		case GL_FLOAT_VEC2:
			attributeCount = 2;
			break;

		case GL_FLOAT_VEC3:
			attributeCount = 3;
			break;

		case GL_FLOAT_VEC4:
			attributeCount = 4;
			break;
		default:
			break;
		}

		program.layout.attributes.push_back(VertexShaderAttribute(attributeLocation, attributeCount));
		delete[] name;
	}

	return ret;
}


u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
	String programSource = ReadTextFile(filepath);

	Program program = {};
	program.handle = CreateProgramFromSource(programSource, programName);
	program.filepath = filepath;
	program.programName = programName;
	program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
	app->programs.push_back(program);

	return app->programs.size() - 1;
}


Image LoadImage(const char* filename)
{
	Image img = {};
	stbi_set_flip_vertically_on_load(true);
	img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
	if (img.pixels)
	{
		img.stride = img.size.x * img.nchannels;
	}
	else
	{
		ELOG("Could not open file %s", filename);
	}
	return img;
}


void FreeImage(Image image)
{
	stbi_image_free(image.pixels);
}


u32 CreateTexture2DFromImage(Image image)
{
	GLenum internalFormat = GL_RGB8;
	GLenum dataFormat = GL_RGB;
	GLenum dataType = GL_UNSIGNED_BYTE;

	switch (image.nchannels)
	{
	case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
	case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
	default: ELOG("LoadTexture2D() - Unsupported number of channels");
	}

	GLuint texHandle;
	glGenTextures(1, &texHandle);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
	for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
		if (app->textures[texIdx].filepath == filepath)
			return texIdx;

	Image image = LoadImage(filepath);

	if (image.pixels)
	{
		Texture tex = {};
		tex.handle = CreateTexture2DFromImage(image);
		tex.filepath = filepath;

		u32 texIdx = app->textures.size();
		app->textures.push_back(tex);

		FreeImage(image);
		return texIdx;
	}
	else
	{
		return UINT32_MAX;
	}
}


//Init------------------------------------------------------------------
void Init(App* app)
{
	GetAppInfo(app);

	InitRect(app);
	InitPrograms(app);
	InitTextures(app);
	InitScene(app);
	InitUniformBuffers(app);
	InitFramebuffer(app);
	InitBloomResources(app);
	InitBloomPrograms(app);
	InitCubemap(app);

	if (GLVersion.major > 4 ||  (GLVersion.major == 4 && GLVersion.minor >= 3))
	{
		glDebugMessageCallback(OnGlError, app);
	}
	
	app->skybox = new Environment(app, "neon_photostudio_4k.hdr");

	app->mode = Mode_Model;
}


void GetAppInfo(App* app)
{
	app->info.version = (char*)glGetString(GL_VERSION);
	app->info.render = (char*)glGetString(GL_RENDERER);
	app->info.vendor = (char*)glGetString(GL_VENDOR);
	app->info.shadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	int extensionCount;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);

	for (int i = 0; i < extensionCount; ++i)
	{
		app->info.extensions.push_back((char*)glGetStringi(GL_EXTENSIONS, i));
	}
}


void InitRect(App* app)
{
	//Create screen rect
	Model screenRect = {};
	Mesh mesh = {};

	VertexBufferLayout vertexLayout = {};
	vertexLayout.stride = 5 * sizeof(float);
	vertexLayout.attributes.push_back(VertexBufferAttribute(0, 3, 0));
	vertexLayout.attributes.push_back(VertexBufferAttribute(2, 2, 3 * sizeof(float)));

	// - vertex buffers
	glGenBuffers(1, &app->embeddedVertices);
	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// - element/index buffers
	glGenBuffers(1, &app->embeddedElements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndices), rectIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// - vaos
	glGenVertexArrays(1, &app->vao);
	glBindVertexArray(app->vao);
	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
	glBindVertexArray(0);
}


void InitPrograms(App* app)
{
	// - programs (and retrieve uniform indices)
	app->screenRectProgramIdx = CreateProgram(app, "texturedQuad.glsl", "TEXTURED_QUAD");
	app->albedoTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "albedo");	//TODO ask what to do about these
	app->normalsTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "normals");
	app->worldPositionTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "worldPos");
	app->defaultTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "defaultTexture");
	app->bloomTexure = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "bloom");
	app->depthTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "depth");
	app->reflectivityTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "reflectivity");
	app->drawModeUniform = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "drawMode");

	app->texturedGeometryProgramIdx = CreateProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
	app->geometryUniformTexture = glGetUniformLocation(app->programs[app->texturedGeometryProgramIdx].handle, "uTexture");

	app->lightProgramIdx = CreateProgram(app, "lightPass.glsl", "LIGHT_PASS");
}


void InitTextures(App* app)
{
	// - textures
	app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
	app->diceTexIdx = LoadTexture2D(app, "dice.png");
	app->blackTexIdx = LoadTexture2D(app, "color_black.png");
	app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
	app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");
}


void InitScene(App* app)
{
	LoadModel(app, "Patrick/Patrick.obj", true);
	if (app->entities.size() != 0)
	{
		app->entities[0].scale = glm::vec3(0.4f, 0.4f, 0.4f);
		app->entities[0].position = glm::vec3(0.0f, 1.9f, 0.6f);
	}

	//LoadModel(app, "Room/Room #1.obj", true);
	app->sphereModel = LoadModel(app, "DefaultShapes/Sphere.fbx");
	app->planeModel = LoadPlane(app);

	app->lights.push_back(Light(LIGHT_TYPE::POINT, glm::vec3(0.9, 0.7, 0.6), glm::vec3(0.0, -1.0, 0.0), glm::vec3(3.f, 4.f, 3.f)));
	app->lights.push_back(Light(LIGHT_TYPE::POINT, glm::vec3(0.9, 0.7, 0.6), glm::vec3(0.0, -1.0, 0.0), glm::vec3(-3.f, 4.f, 3.f)));
	app->lights.push_back(Light(LIGHT_TYPE::POINT, glm::vec3(0.9, 0.7, 0.6), glm::vec3(0.0, -1.0, 0.0), glm::vec3(3.f, 4.f, -3.f)));
	app->lights.push_back(Light(LIGHT_TYPE::POINT, glm::vec3(0.9, 0.7, 0.6), glm::vec3(0.0, -1.0, 0.0), glm::vec3(-3.f, 4.f, -3.f)));
}


void InitUniformBuffers(App* app)
{
	//uniform buffer
	int maxUniformBufferSize;
	int uniformAlignment;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformAlignment);

	app->localUniformBuffer = CreateBuffer(maxUniformBufferSize, uniformAlignment, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
	app->debugLightUniformBuffer = CreateBuffer(maxUniformBufferSize, uniformAlignment, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
	app->materialUniformBuffer = CreateBuffer(maxUniformBufferSize, uniformAlignment, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
	app->globalUniformBuffer = CreateBuffer(maxUniformBufferSize, uniformAlignment, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
}




void InitFramebuffer(App* app)
{
	//Generate framebuffer
	//Albedo
	app->framebuffer.PushTexture(app->displaySize.x, app->displaySize.y, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	//Normals
	app->framebuffer.PushTexture(app->displaySize.x, app->displaySize.y, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE);

	//World Pos
	app->framebuffer.PushTexture(app->displaySize.x, app->displaySize.y, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE);

	//Default
	app->framebuffer.PushTexture(app->displaySize.x, app->displaySize.y, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	//Bloom
	app->framebuffer.PushTexture(app->displaySize.x, app->displaySize.y, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE);

	//Reflectivity
	app->framebuffer.PushTexture(app->displaySize.x, app->displaySize.y, GL_R16F, GL_RED, GL_FLOAT);
	
	//Depth
	app->framebuffer.PushTexture(app->displaySize.x, app->displaySize.y, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);

	app->framebuffer.AttachTextures();
}


void InitBloomResources(App* app)
{
	app->fboBloom1.ClearColorAttachments();
	app->fboBloom2.ClearColorAttachments();
	app->fboBloom3.ClearColorAttachments();
	app->fboBloom4.ClearColorAttachments();
	app->fboBloom5.ClearColorAttachments();

	//Bright mipmap
	if (app->rtBright != 0)
		glDeleteTextures(1, &app->rtBright);

	glGenTextures(1, &app->rtBright);
	glBindTexture(GL_TEXTURE_2D, app->rtBright);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app->displaySize.x / 2,  app->displaySize.y / 2,  0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, app->displaySize.x / 4,  app->displaySize.y / 4,  0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA16F, app->displaySize.x / 8,  app->displaySize.y / 8,  0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA16F, app->displaySize.x / 16, app->displaySize.y / 16, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA16F, app->displaySize.x / 32, app->displaySize.y / 32, 0, GL_RGBA, GL_FLOAT, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Bloom mipmap
	if (app->rtBloom != 0)
		glDeleteTextures(1, &app->rtBloom);

	glGenTextures(1, &app->rtBloom);
	glBindTexture(GL_TEXTURE_2D, app->rtBloom);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app->displaySize.x / 2, app->displaySize.y / 2, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, app->displaySize.x / 4, app->displaySize.y / 4, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA16F, app->displaySize.x / 8, app->displaySize.y / 8, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA16F, app->displaySize.x / 16, app->displaySize.y / 16, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA16F, app->displaySize.x / 32, app->displaySize.y / 32, 0, GL_RGBA, GL_FLOAT, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);


	glBindTexture(GL_TEXTURE_2D, 0);

	//FBO 1
	app->fboBloom1.Create();
	app->fboBloom1.Bind();
	app->fboBloom1.PushColorAttachment(0, app->rtBright, 0);
	app->fboBloom1.PushColorAttachment(1, app->rtBloom, 0);
	app->fboBloom1.Unbind();
	app->fboBloom1.CheckStatus();


	//FBO 2
	app->fboBloom2.Create();
	app->fboBloom2.Bind();
	app->fboBloom2.PushColorAttachment(0, app->rtBright, 1);
	app->fboBloom2.PushColorAttachment(1, app->rtBloom, 1);
	app->fboBloom2.Unbind();
	app->fboBloom2.CheckStatus();


	//FBO 3
	app->fboBloom3.Create();
	app->fboBloom3.Bind();
	app->fboBloom3.PushColorAttachment(0, app->rtBright, 2);
	app->fboBloom3.PushColorAttachment(1, app->rtBloom, 2);
	app->fboBloom3.Unbind();
	app->fboBloom3.CheckStatus();

	//FBO 4
	app->fboBloom4.Create();
	app->fboBloom4.Bind();
	app->fboBloom4.PushColorAttachment(0, app->rtBright, 3);
	app->fboBloom4.PushColorAttachment(1, app->rtBloom, 3);
	app->fboBloom4.Unbind();
	app->fboBloom4.CheckStatus();

	//FBO 5
	app->fboBloom5.Create();
	app->fboBloom5.Bind();
	app->fboBloom5.PushColorAttachment(0, app->rtBright, 4);
	app->fboBloom5.PushColorAttachment(1, app->rtBloom, 4);
	app->fboBloom5.Unbind();
	app->fboBloom5.CheckStatus();
}


void InitBloomPrograms(App* app)
{
	app->brightPixelProgramIdx = CreateProgram(app, "BrightPixelDetection.glsl", "BRIGHT_DETECTION");
	app->bloomBlurrProgramIdx = CreateProgram(app, "BloomBlurrPass.glsl", "BLURR_BLOOM");
	app->bloomProgramIdx = CreateProgram(app, "BloomPass.glsl", "BLOOM_PASS");
}


void InitCubemap(App* app)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}


//GUI------------------------------------------------------------------------------------------
void Gui(App* app)
{
	ImGui::Begin("Window");

	DrawModeGui(app);

	ImGui::Checkbox("Debug draw lights", &app->debugDrawLights);
	
	DrawInfoGui(app);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	DrawModelListGui(app);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	DrawEntityListGui(app);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	DrawCameraGui(app);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	DrawLightGui(app);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	DrawBloomGui(app);

	DrawEntityGui(app);
	
	ImGui::End();
}

void DrawModeGui(App* app)
{
	if (ImGui::BeginMenu("Draw mode"))
	{
		if (ImGui::MenuItem("Default"))
			app->drawMode = DRAW_MODE::DEFAULT;

		if (ImGui::MenuItem("Albedo"))
			app->drawMode = DRAW_MODE::ALBEDO;

		if (ImGui::MenuItem("Normals"))
			app->drawMode = DRAW_MODE::NORMALS;

		if (ImGui::MenuItem("World position"))
			app->drawMode = DRAW_MODE::WORLD_POS;

		if (ImGui::MenuItem("Bloom"))
			app->drawMode = DRAW_MODE::BLOOM;

		if (ImGui::MenuItem("Reflectivity"))
			app->drawMode = DRAW_MODE::REFLECTIVITY;

		if (ImGui::MenuItem("Depth"))
			app->drawMode = DRAW_MODE::DEPTH;

		ImGui::EndMenu();
	}
}

void DrawInfoGui(App* app)
{
	if (ImGui::CollapsingHeader("Info", ImGuiTreeNodeFlags_None))
	{
		ImGui::Text("FPS: %f", 1.0f / app->deltaTime);

		if (ImGui::BeginMenu("RenderMode"))
		{
			if (ImGui::MenuItem("Textured quad"))
				app->mode = Mode_TexturedQuad;

			if (ImGui::MenuItem("Model"))
				app->mode = Mode_Model;

			ImGui::EndMenu();
		}

		ImGui::Separator();
		ImGui::Text("OpenGl version: %s", app->info.version.c_str());
		ImGui::Text("GPU: %s", app->info.render.c_str());
		ImGui::Text("Vendor: %s", app->info.vendor.c_str());
		ImGui::Text("Shading language version: %s", app->info.shadingLanguageVersion.c_str());

		int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

		bool open = ImGui::TreeNodeEx("Extensions", flags);

		if (open == true)
		{
			int extensionCount = app->info.extensions.size();
			flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf;

			for (int i = 0; i < extensionCount; ++i)
			{
				ImGui::TreeNodeEx(app->info.extensions[i].c_str(), flags);
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
}


void DrawModelListGui(App* app)
{
	if (ImGui::CollapsingHeader("Model list", ImGuiTreeNodeFlags_None))
	{
		int modelCount = app->models.size();
		for (int i = 0; i < modelCount; ++i)
		{
			if (ImGui::Button(app->models[i].name.c_str()))
			{
				app->entityIdCount++;
				app->entities.push_back(Entity(app->models[i].name + std::to_string(app->entityIdCount), i));
			}
		}
	}
}


void DrawEntityListGui(App* app)
{
	if (ImGui::CollapsingHeader("Entity list", ImGuiTreeNodeFlags_None))
	{
		int entityCount = app->entities.size();
		for (int i = 0; i < entityCount; ++i)
		{
			ImGui::PushID(i);
			if (ImGui::Button(app->entities[i].name.c_str()))
			{
				app->entities[i].drawInspector = !app->entities[i].drawInspector;
			}
			ImGui::PopID();
		}
	}
}


void DrawEntityGui(App* app)
{
	for (int i = 0; i < app->entities.size(); ++i)
	{
		bool deleteEnity = false;

		if (app->entities[i].drawInspector == true)
		{
			Entity& entity = app->entities[i];
			
			ImGui::PushID(i);
			ImGui::Begin(entity.name.c_str(), &entity.drawInspector);

			ImGui::NewLine();

			ImGui::DragFloat3("Position", &entity.position.x, 0.05f);
			ImGui::DragFloat3("Rotation", &entity.rotation.x, 0.05f);
			ImGui::DragFloat3("Scale", &entity.scale.x, 0.05f);

			ImGui::NewLine();
			
			if (ImGui::Button("Delete entity"))
				deleteEnity = true;

			ImGui::NewLine();

			Model& model = app->models[app->entities[i].modelIdx];

			int materialCount = model.materialIdx.size();
			for (int j = 0; j < materialCount; ++j)
			{
				Material& mat = app->materials[model.materialIdx[j]];
				if (ImGui::CollapsingHeader(mat.name.c_str(), ImGuiTreeNodeFlags_None))
				{
					ImGui::PushID(j);
					ImGui::Spacing();	ImGui::Spacing();

					ImGui::InputFloat3("Albedo", &mat.albedo.x, "%.1f", ImGuiInputTextFlags_AutoSelectAll);
					ImGui::Spacing();
					ImGui::InputFloat3("Emissive", &mat.emissive.x, "%.1f", ImGuiInputTextFlags_AutoSelectAll);
					ImGui::Spacing();
					ImGui::DragFloat("Smoothness", &mat.smoothness, 0.01f, 0.0f, 100.0f);
					ImGui::Spacing();
					ImGui::DragFloat("Reflectivity", &mat.reflectivity, 0.005f, 0.0f, 1.0f);

					ImVec2 textureSize = ImVec2(124, 124);

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Albedo texture");

					if (mat.albedoTextureIdx != UINT32_MAX)
						ImGui::Image((ImTextureID)app->textures[mat.albedoTextureIdx].handle, textureSize);

					else
						ImGui::Text("No texture");

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Emissive texture");

					if (mat.emissiveTextureIdx != UINT32_MAX)
						ImGui::Image((ImTextureID)app->textures[mat.emissiveTextureIdx].handle, textureSize);

					else
						ImGui::Text("No texture");

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Specular texture");

					if (mat.specularTextureIdx != UINT32_MAX)
						ImGui::Image((ImTextureID)app->textures[mat.specularTextureIdx].handle, textureSize);

					else
						ImGui::Text("No texture");

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Normals texture");

					if (mat.normalsTextureIdx != UINT32_MAX)
						ImGui::Image((ImTextureID)app->textures[mat.normalsTextureIdx].handle, textureSize);
					
					else
						ImGui::Text("No texture");

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Bump texture");
					if (mat.bumpTextureIdx != UINT32_MAX)
						ImGui::Image((ImTextureID)app->textures[mat.bumpTextureIdx].handle, textureSize);

					else
						ImGui::Text("No texture");

					ImGui::Spacing();	ImGui::Spacing();
					ImGui::PopID();
				}
			}

			ImGui::End();
			ImGui::PopID();
		}

		if (deleteEnity == true)
		{
			app->entities.erase(app->entities.begin() + i);
			i--;
		}
	}
}


void DrawCameraGui(App* app)
{
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_None))
	{
		ImGui::NewLine();

		ImGui::DragFloat3("Position", app->camera.GetPosition(), 0.05f);
		ImGui::DragFloat3("Target pos", app->camera.GetTarget(), 0.05f);

		ImGui::NewLine();

		ImGui::DragFloat("FOV", app->camera.GetFOV(), 0.05f);
		ImGui::DragFloat("Z Near", app->camera.GetZNear(), 0.001f, 0.f);
		ImGui::DragFloat("Z Far", app->camera.GetZFar(), 0.6f);
	}
}


void DrawLightGui(App* app)
{
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_None))
	{
		ImGui::NewLine();

		if (ImGui::Button("Create point light"))
		{
			app->lights.push_back(Light(LIGHT_TYPE::POINT, glm::vec3(0.9, 0.8, 0.8), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0)));
		}

		ImGui::NewLine();

		if (ImGui::Button("Create directional light"))
		{
			app->lights.push_back(Light(LIGHT_TYPE::DIRECTIONAL, glm::vec3(0.9, 0.8, 0.8), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0)));
		}

		ImGui::NewLine();

		ImGui::InputFloat("Ambient light strength", &app->ambientLightStrength);
		ImGui::NewLine();
		ImGui::ColorPicker3("Ambient light color", glm::value_ptr(app->ambientLightColor));

		ImGui::NewLine();

		char lightNameBuffer[100];

		for (int i = 0; i < app->lights.size(); ++i)
		{
			sprintf_s(lightNameBuffer, 100, "Light %i", i);
			ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), lightNameBuffer);

			ImGui::PushID(i);

			if (ImGui::Button("Destroy Light"))
			{
				app->lights.erase(app->lights.begin() + i);
				i--;
				ImGui::PopID();
				continue;
			}
			
			switch (app->lights[i].type)
			{
			case LIGHT_TYPE::DIRECTIONAL:
				ImGui::Text("Directional");

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::DragFloat3("Direction", glm::value_ptr(app->lights[i].direction), 0.2f);
				break;

			case LIGHT_TYPE::POINT:
				ImGui::Text("Point");

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::DragFloat3("Position", glm::value_ptr(app->lights[i].position), 0.2f);
				ImGui::DragFloat("Max distance", &app->lights[i].maxDistance, 0.05f);

				if (app->lights[i].maxDistance < 0.0)
					app->lights[i].maxDistance = 0.0;

				break;

			default:
				ELOG("Need to add light type to ui switch");
				break;
			}

			ImGui::ColorPicker3("Color", glm::value_ptr(app->lights[i].color));

			ImGui::PopID();

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
		}
	}
}


void DrawBloomGui(App* app)
{
	if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_None))
	{
		ImGui::NewLine();
		
		ImGui::Checkbox("Apply bloom", &app->applyBloom);

		ImGui::NewLine();

		ImGui::SliderFloat("LOD 1 intensity:", &app->bloomIntensity1, 0.0f, 1.0f);
		ImGui::SliderFloat("LOD 2 intensity:", &app->bloomIntensity2, 0.0f, 1.0f);
		ImGui::SliderFloat("LOD 3 intensity:", &app->bloomIntensity3, 0.0f, 1.0f);
		ImGui::SliderFloat("LOD 4 intensity:", &app->bloomIntensity4, 0.0f, 1.0f);
		ImGui::SliderFloat("LOD 5 intensity:", &app->bloomIntensity5, 0.0f, 1.0f);

		ImGui::NewLine();
	}
}


//Update----------------------------------------------------------------------------
void Update(App* app)
{
	// You can handle app->input keyboard/mouse here
	CheckToUpdateShaders(app);

	UpdateCamera(app);

	FillUniformGlobalParams(app);
	FillUniformDebugLightParams(app);
	FillUniformMaterialParams(app);
	FillUniformLocalParams(app);
}


void CheckToUpdateShaders(App* app)
{
	int programCount = app->programs.size();

	for (int i = 0; i < programCount; ++i)
	{
		std::string path = app->programs[i].filepath;
		u64 currentTimeStamp = GetFileLastWriteTimestamp(path.c_str());

		if (app->programs[i].lastWriteTimestamp < currentTimeStamp)
		{
			glDeleteProgram(app->programs[i].handle);

			String source = ReadTextFile(app->programs[i].filepath.c_str());
			app->programs[i].handle = CreateProgramFromSource(source, app->programs[i].programName.c_str());
			app->programs[i].lastWriteTimestamp = currentTimeStamp;
		}
	}
}


void UpdateCamera(App* app)
{
	if (app->displaySize.x != 0 && app->displaySize.y != 0)
	{
		app->camera.SetAspectRatio(app->displaySize.x / app->displaySize.y);
	}

	app->camera.HandleInput(&app->input);
}


void FillUniformLocalParams(App* app)
{
	BindBuffer(app->localUniformBuffer);
	MapBuffer(app->localUniformBuffer, GL_WRITE_ONLY);

	glm::mat4 projection = app->camera.GetProjectionMatrix();
	glm::mat4 view = app->camera.GetViewMatrix();

	int entityCount = app->entities.size();
	for (int i = 0; i < entityCount; ++i)
	{
		AlignHead(app->localUniformBuffer, app->localUniformBuffer.alignement);

		app->entities[i].localParamsOffset = app->localUniformBuffer.head;
		
		glm::mat4 worldTransform = app->entities[i].CalculateWorldTransform();
		PushMat4(app->localUniformBuffer, worldTransform);

		glm::mat4 worldViewProjection = projection * view * worldTransform;
		PushMat4(app->localUniformBuffer, worldViewProjection);

		app->entities[i].localParamsSize = app->localUniformBuffer.head - app->entities[i].localParamsOffset;
	}

	UnmapBuffer(app->localUniformBuffer);
}


void FillUniformDebugLightParams(App* app)
{
	BindBuffer(app->debugLightUniformBuffer);
	MapBuffer(app->debugLightUniformBuffer, GL_WRITE_ONLY);

	glm::mat4 projection = app->camera.GetProjectionMatrix();
	glm::mat4 view = app->camera.GetViewMatrix();

	int lightCount = app->lights.size();
	for (int i = 0; i < lightCount; ++i)
	{
		AlignHead(app->debugLightUniformBuffer, app->debugLightUniformBuffer.alignement);

		app->lights[i].localParamsOffset = app->debugLightUniformBuffer.head;

		glm::mat4 worldTransform = app->lights[i].CalculateWorldTransform();
		PushMat4(app->debugLightUniformBuffer, worldTransform);

		glm::mat4 worldViewProjection = projection * view * worldTransform;
		PushMat4(app->debugLightUniformBuffer, worldViewProjection);

		app->lights[i].localParamsSize = app->debugLightUniformBuffer.head - app->lights[i].localParamsOffset;
	}

	UnmapBuffer(app->debugLightUniformBuffer);
}


void FillUniformMaterialParams(App* app)
{
	BindBuffer(app->materialUniformBuffer);
	MapBuffer(app->materialUniformBuffer, GL_WRITE_ONLY);

	int materialCount = app->materials.size();
	for (int i = 0; i < materialCount; ++i)
	{
		AlignHead(app->materialUniformBuffer, app->materialUniformBuffer.alignement);

		app->materials[i].localParamsOffset = app->materialUniformBuffer.head;

		PushVec3(app->materialUniformBuffer, app->materials[i].albedo);

		PushVec3(app->materialUniformBuffer, app->materials[i].emissive);

		PushFloat(app->materialUniformBuffer, app->materials[i].reflectivity);

		app->materials[i].localParamsSize = app->materialUniformBuffer.head - app->materials[i].localParamsOffset;
	}

	UnmapBuffer(app->materialUniformBuffer);
}


void FillUniformGlobalParams(App* app)
{
	BindBuffer(app->globalUniformBuffer);
	MapBuffer(app->globalUniformBuffer, GL_WRITE_ONLY);
	
	app->globalParamsOffset = app->globalUniformBuffer.head;

	PushVec3(app->globalUniformBuffer, app->camera.GetPositionV3());
	PushUInt(app->globalUniformBuffer, app->lights.size());

	PushFloat(app->globalUniformBuffer,app->ambientLightStrength);
	PushVec3(app->globalUniformBuffer, app->ambientLightColor);

	int lightCount = app->lights.size();
	for (int i = 0; i < lightCount; ++i)
	{
		AlignHead(app->globalUniformBuffer, sizeof(glm::vec4));

		PushUInt(app->globalUniformBuffer, (u32)app->lights[i].type);
		PushFloat(app->globalUniformBuffer, app->lights[i].maxDistance);
		PushVec3(app->globalUniformBuffer, app->lights[i].color);
		PushVec3(app->globalUniformBuffer, app->lights[i].direction);
		PushVec3(app->globalUniformBuffer, app->lights[i].position);
	}

	app->globalParamsSize = app->globalUniformBuffer.head - app->globalParamsOffset;

	UnmapBuffer(app->globalUniformBuffer);
}


//Render----------------------------------------------------------------------------
void Render(App* app)
{
	switch (app->mode)
	{
	case Mode_Model:
	{
		RenderModels(app);

		if (app->debugDrawLights == true)
			DebugDrawLights(app);

		app->skybox->RenderSkybox(app);
		LightPass(app);


		if (app->applyBloom == true)
			BloomPass(app);

		RenderScene(app);
	}
	break;

	default:
		break;
	}
}


u32 FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
	Submesh& submesh = mesh.submeshes[submeshIndex];

	int vaoCount = submesh.vaos.size();

	for (int i = 0; i < vaoCount; ++i)
	{
		if (submesh.vaos[i].programHandle == program.handle)
			return submesh.vaos[i].handle;
	}

	u32 vaoHandle = 0;

	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

	int vAttribCount = program.layout.attributes.size();

	for (int i = 0; i < vAttribCount; ++i)
	{
		bool attribLinked = false;

		int bAttribCount = submesh.vertexBufferLayout.attributes.size();
		for (int j = 0; j < bAttribCount; ++j)
		{
			if (program.layout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
			{
				u32 index = submesh.vertexBufferLayout.attributes[j].location;
				u32 nComp = submesh.vertexBufferLayout.attributes[j].componentCount;
				u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;
				u32 stride = submesh.vertexBufferLayout.stride;

				glVertexAttribPointer(index, nComp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
				glEnableVertexAttribArray(index);

				attribLinked = true;
				break;
			}
		}

		ELOG("Missed attribute link in mesh");
		//assert(attribLinked, "Missed attribute link");
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	submesh.vaos.push_back(Vao(vaoHandle, program.handle));
	return vaoHandle;
}


void RenderModels(App* app)
{
	glBindFramebuffer(GL_FRAMEBUFFER, app->framebuffer.handle);

	u32 drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

	glClearColor(0.f, 0.f, 0.f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	Program programTexGeo = app->programs[app->texturedGeometryProgramIdx];
	glUseProgram(programTexGeo.handle);

	int entityCount = app->entities.size();
	for (int i = 0; i < entityCount; ++i)
	{
		Model& model = app->models[app->entities[i].modelIdx];
		Mesh& mesh = app->meshes[model.meshIdx];

		int submeshCount = mesh.submeshes.size();

		glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->localUniformBuffer.handle, app->entities[i].localParamsOffset, app->entities[i].localParamsSize);

		for (int j = 0; j < submeshCount; ++j)
		{
			u32 vao = FindVAO(mesh, j, programTexGeo);
			glBindVertexArray(vao);

			u32 materialIdx = model.materialIdx[j];
			Material& material = app->materials[materialIdx];

			glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(2), app->materialUniformBuffer.handle, material.localParamsOffset, material.localParamsSize);

			if (material.albedoTextureIdx != UINT32_MAX)
			{
				glBindTexture(GL_TEXTURE_2D, app->textures[material.albedoTextureIdx].handle);
				glUniform1i(app->geometryUniformTexture, 0);
				glActiveTexture(GL_TEXTURE0);
			}
			
			Submesh& submesh = mesh.submeshes[j];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}
	}

	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void DebugDrawLights(App* app)
{
	glBindFramebuffer(GL_FRAMEBUFFER, app->framebuffer.handle);

	u32 drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

	glEnable(GL_DEPTH_TEST);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	Program programTexGeo = app->programs[app->texturedGeometryProgramIdx];
	glUseProgram(programTexGeo.handle);

	int lightCount = app->lights.size();
	for (int i = 0; i < lightCount; ++i)
	{
		int modelIdx = 0;

		switch (app->lights[i].type)
		{
		case LIGHT_TYPE::DIRECTIONAL :
			modelIdx = app->planeModel;
				break;

		case LIGHT_TYPE::POINT :
			modelIdx = app->sphereModel;
			break;

		default:
			ELOG("Need to add light type");
			break;
		}

		Model& model = app->models[modelIdx];
		Mesh& mesh = app->meshes[model.meshIdx];

		glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->debugLightUniformBuffer.handle, app->lights[i].localParamsOffset, app->lights[i].localParamsSize);
		
		int submeshCount = mesh.submeshes.size();

		for (int j = 0; j < submeshCount; ++j)
		{
			u32 vao = FindVAO(mesh, j, programTexGeo);
			glBindVertexArray(vao);

			u32 materialIdx = model.materialIdx[j];
			Material& material = app->materials[materialIdx];

			if (material.albedoTextureIdx != UINT32_MAX)
			{
				glBindTexture(GL_TEXTURE_2D, app->textures[material.albedoTextureIdx].handle);
				glUniform1i(app->geometryUniformTexture, 0);
				glActiveTexture(GL_TEXTURE0);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, app->textures[app->whiteTexIdx].handle);
				glUniform1i(app->geometryUniformTexture, 0);
				glActiveTexture(GL_TEXTURE0);
			}

			Submesh& submesh = mesh.submeshes[j];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}
	}

	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void LightPass(App* app)
{
	glBindFramebuffer(GL_FRAMEBUFFER, app->framebuffer.handle);

	u32 drawBuffers[] = { GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

	glDisable(GL_DEPTH_TEST);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	Program programTexGeo = app->programs[app->lightProgramIdx];
	glUseProgram(programTexGeo.handle);
	glBindVertexArray(app->vao);

	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->globalUniformBuffer.handle, app->globalParamsOffset, app->globalParamsSize);

	GLuint alb = glGetUniformLocation(app->programs[app->lightProgramIdx].handle, "albedo");	//TODO ask what to do about these
	GLuint norm = glGetUniformLocation(app->programs[app->lightProgramIdx].handle, "normals");
	GLuint pos = glGetUniformLocation(app->programs[app->lightProgramIdx].handle, "worldPos");
	GLuint reflx = glGetUniformLocation(app->programs[app->lightProgramIdx].handle, "reflectivity");
	GLuint skyBox = glGetUniformLocation(app->programs[app->lightProgramIdx].handle, "skyBox");
	GLuint irr = glGetUniformLocation(app->programs[app->lightProgramIdx].handle, "irradianceMap");

	// - bind the texture into unit 0
	glUniform1i(alb, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[0].handle);

	glUniform1i(norm, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[1].handle);

	glUniform1i(pos, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[2].handle);
	
	glUniform1i(reflx, 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[5].handle);

	glUniform1i(skyBox, 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, app->skybox->cubeMap.handle);

	glUniform1i(irr, 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, app->skybox->irradianceMap.handle);



	// - draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// - clean
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void BloomPass(App* app)
{
	BrightPixelPass(app);
	glBindTexture(GL_TEXTURE_2D, app->rtBright);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	BlurrBloomPass(app);

	ApplyBloomPass(app);
}


void RenderScene(App* app)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	Program programTexGeo = app->programs[app->screenRectProgramIdx];
	glUseProgram(programTexGeo.handle);
	glBindVertexArray(app->vao);

	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->globalUniformBuffer.handle, app->globalParamsOffset, app->globalParamsSize);

	// - bind the texture into unit 0
	glUniform1i(app->albedoTexture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[0].handle);

	glUniform1i(app->normalsTexture, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[1].handle);

	glUniform1i(app->worldPositionTexture, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[2].handle);

	glUniform1i(app->defaultTexture, 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[3].handle);

	glUniform1i(app->bloomTexure, 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[4].handle);

	glUniform1i(app->reflectivityTexture, 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[5].handle);

	glUniform1i(app->depthTexture, 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[6].handle);

	glUniform1i(app->drawModeUniform, (int)app->drawMode);

	// - draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// - clean
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}


void BrightPixelPass(App* app)
{
	app->fboBloom1.CheckStatus();
	glBindFramebuffer(GL_FRAMEBUFFER, app->fboBloom1.handle);

	u32 drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x * 0.5, app->displaySize.y * 0.5);
	glDisable(GL_DEPTH_TEST);

	// - bind program
	Program program = app->programs[app->brightPixelProgramIdx];
	glUseProgram(program.handle);
	glBindVertexArray(app->vao);

	GLuint alb = glGetUniformLocation(program.handle, "albedoTexture");

	// - bind the texture into unit 0
	glUniform1i(alb, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[0].handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLuint uniformLoc = glGetUniformLocation(program.handle, "threshold");
	glUniform1f(uniformLoc, 0.99f);

	// - draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void BlurrBloomPass(App* app)
{
	//horizontal blurr
	Blurr(app, app->fboBloom1, app->displaySize.x / 2,  app->displaySize.y / 2,  1, app->rtBright, 0, 1.f, 0.f);
	Blurr(app, app->fboBloom2, app->displaySize.x / 4,  app->displaySize.y / 4,  1, app->rtBright, 1, 1.f, 0.f);
	Blurr(app, app->fboBloom3, app->displaySize.x / 8,  app->displaySize.y / 8,  1, app->rtBright, 2, 1.f, 0.f);
	Blurr(app, app->fboBloom4, app->displaySize.x / 16, app->displaySize.y / 16, 1, app->rtBright, 3, 1.f, 0.f);
	Blurr(app, app->fboBloom5, app->displaySize.x / 32, app->displaySize.y / 32, 1, app->rtBright, 4, 1.f, 0.f);

	//Vertical blurr
	Blurr(app, app->fboBloom1, app->displaySize.x / 2,  app->displaySize.y / 2,  0, app->rtBloom, 0, 0.f, 1.f);
	Blurr(app, app->fboBloom2, app->displaySize.x / 4,  app->displaySize.y / 4,  0, app->rtBloom, 1, 0.f, 1.f);
	Blurr(app, app->fboBloom3, app->displaySize.x / 8,  app->displaySize.y / 8,  0, app->rtBloom, 2, 0.f, 1.f);
	Blurr(app, app->fboBloom4, app->displaySize.x / 16, app->displaySize.y / 16, 0, app->rtBloom, 3, 0.f, 1.f);
	Blurr(app, app->fboBloom5, app->displaySize.x / 32, app->displaySize.y / 32, 0, app->rtBloom, 4, 0.f, 1.f);
}


void Blurr(App* app, FrameBuffer& fbo, int texSizeX, int texSizeY, int attachment, u32 texture, int LOD, float directionX, float directionY)
{
	glDisable(GL_DEPTH_TEST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.handle);
	
	u32 drawBuffers[] = { GL_COLOR_ATTACHMENT0 + attachment };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);
	glViewport(0, 0, texSizeX, texSizeY);

	Program program = app->programs[app->bloomBlurrProgramIdx];
	glUseProgram(program.handle);

	glBindVertexArray(app->vao);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	u32 uniformLoc = glGetUniformLocation(program.handle, "colorMap");
	glUniform1i(uniformLoc, 0);

	uniformLoc = glGetUniformLocation(program.handle, "direction");
	glUniform2f(uniformLoc, directionX, directionY);

	uniformLoc = glGetUniformLocation(program.handle, "inputLod");
	glUniform1i(uniformLoc, LOD);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void ApplyBloomPass(App* app)
{
	glBindFramebuffer(GL_FRAMEBUFFER, app->framebuffer.handle);

	u32 drawBuffers[] = {GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	Program program = app->programs[app->bloomProgramIdx];
	glUseProgram(program.handle);
	glBindVertexArray(app->vao);

	GLuint uniformLocation = glGetUniformLocation(program.handle, "bloomMap");

	// - bind the texture into unit 0
	glUniform1i(uniformLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, app->rtBright);

	uniformLocation = glGetUniformLocation(program.handle, "colorMap");

	// - bind the texture into unit 0
	glUniform1i(uniformLocation, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.textures[3].handle);

	uniformLocation = glGetUniformLocation(program.handle, "maxLod");
	glUniform1i(uniformLocation, 4);

	uniformLocation = glGetUniformLocation(program.handle, "lodIntensity[0]");
	glUniform1f(uniformLocation, app->bloomIntensity1);	
	
	uniformLocation = glGetUniformLocation(program.handle, "lodIntensity[1]");
	glUniform1f(uniformLocation, app->bloomIntensity2);	
	
	uniformLocation = glGetUniformLocation(program.handle, "lodIntensity[2]");
	glUniform1f(uniformLocation, app->bloomIntensity3);	
	
	uniformLocation = glGetUniformLocation(program.handle, "lodIntensity[3]");
	glUniform1f(uniformLocation, app->bloomIntensity4);	
	
	uniformLocation = glGetUniformLocation(program.handle, "lodIntensity[4]");
	glUniform1f(uniformLocation, app->bloomIntensity5);

	// - draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// - clean
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* useParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	ELOG("OpenGL debug message: %s", message);

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:				ELOG(" - source: GL_DEBUG_SOURCE_API"); break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM"); break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:	ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER"); break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:		ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY"); break;
	case GL_DEBUG_SOURCE_APPLICATION:		ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION");	break;
	case GL_DEBUG_SOURCE_OTHER:				ELOG(" - source: GL_DEBUG_SOURCE_OTHER"); break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:				ELOG(" - type: GL_DEBUG_TYPE_ERROR"); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ELOG(" - type: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	ELOG(" - type: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_PORTABILITY:			ELOG(" - type: GL_DEBUG_TYPE_PORTABILITY"); break;
	case GL_DEBUG_TYPE_PERFORMANCE:			ELOG(" - type: GL_DEBUG_TYPE_PERFORMANCE"); break;
	case GL_DEBUG_TYPE_MARKER:				ELOG(" - type: GL_DEBUG_TYPE_MARKER"); break;
	case GL_DEBUG_TYPE_PUSH_GROUP:			ELOG(" - type: GL_DEBUG_TYPE_PUSH_GROUP"); break;
	case GL_DEBUG_TYPE_POP_GROUP:			ELOG(" - type: GL_DEBUG_TYPE_POP_GROUP"); break;
	case GL_DEBUG_TYPE_OTHER:				ELOG(" - type: GL_DEBUG_TYPE_OTHER");  break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:			ELOG(" - severity: GL_DEBUG_SEVERITY_HIGH"); break;
	case GL_DEBUG_SEVERITY_MEDIUM:			ELOG(" - severity: GL_DEBUG_SEVERITY_MEDIUM"); break;
	case GL_DEBUG_SEVERITY_LOW:				ELOG(" - severity: GL_DEBUG_SEVERITY_LOW"); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	ELOG(" - severity: GL_DEBUG_SEVERITY_NOTIFICATION"); break;
	}

}