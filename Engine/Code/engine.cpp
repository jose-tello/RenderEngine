//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "assimp_model_loading.h"

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


GLuint CreateTexture2DFromImage(Image image)
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

	InitResources(app);

	if (GLVersion.major > 4 ||  (GLVersion.major == 4 && GLVersion.minor >= 3))
	{
		glDebugMessageCallback(OnGlError, app);
	}

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


void InitResources(App* app)
{
	//Create screen rect
	Model screenRect = {};
	Mesh mesh = {};

	VertexBufferLayout vertexLayout = {};
	vertexLayout.stride = 5 * sizeof(float);
	vertexLayout.attributes.push_back(VertexBufferAttribute(0, 3, 0));
	vertexLayout.attributes.push_back(VertexBufferAttribute(2, 2, 3 * sizeof(float)));
	
	LoadModel(app, "Patrick/Patrick.obj");
	//LoadModel(app, "Room/Room #1.obj");

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

	// - programs (and retrieve uniform indices)
	app->screenRectProgramIdx = CreateProgram(app, "texturedQuad.glsl", "TEXTURED_QUAD");
	app->albedoTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "albedo");
	app->normalsTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "normals");
	app->worldPositionTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "worldPos");
	app->depthTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "depth");
	app->drawModeUniform = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "drawMode");

	app->texturedGeometryProgramIdx = CreateProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
	app->geometryUniformTexture = glGetUniformLocation(app->programs[app->texturedGeometryProgramIdx].handle, "uTexture");

	// - textures
	app->diceTexIdx = LoadTexture2D(app, "dice.png");
	app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
	app->blackTexIdx = LoadTexture2D(app, "color_black.png");
	app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
	app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

	//uniform buffer
	int maxUniformBufferSize;
	int uniformAlignment;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformAlignment);

	app->localUniformBuffer = CreateBuffer(maxUniformBufferSize, uniformAlignment, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
	app->globalUniformBuffer = CreateBuffer(maxUniformBufferSize, uniformAlignment, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);

	app->framebuffer.Regenerate(app->displaySize.x, app->displaySize.y);
}


//GUI------------------------------------------------------------------------------------------
void Gui(App* app)
{
	ImGui::Begin("Window");

	DrawModeGui(app);
	
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

					ImVec2 textureSize = ImVec2(124, 124);

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Albedo texture");
					ImGui::Image((ImTextureID)app->textures[mat.albedoTextureIdx].handle, textureSize);

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Emissive texture");
					ImGui::Image((ImTextureID)app->textures[mat.emissiveTextureIdx].handle, textureSize);

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Specular texture");
					ImGui::Image((ImTextureID)app->textures[mat.specularTextureIdx].handle, textureSize);

					ImGui::Spacing();	ImGui::Separator();		ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Normals texture");
					ImGui::Image((ImTextureID)app->textures[mat.normalsTextureIdx].handle, textureSize);

					ImGui::TextColored(ImVec4(0.4, 1.0, 0.4, 1.0), "Bump texture");
					ImGui::Image((ImTextureID)app->textures[mat.bumpTextureIdx].handle, textureSize);

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
		ImGui::DragFloat3("Rotation", app->camera.GetRotation(), 0.05f);

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


//Update----------------------------------------------------------------------------
void Update(App* app)
{
	// You can handle app->input keyboard/mouse here
	CheckToUpdateShaders(app);

	UpdateCamera(app);

	FillUniformGlobalParams(app);
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
	// - clear the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, app->framebuffer.handle);

	u32 drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
			
			glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->localUniformBuffer.handle, app->entities[i].localParamsOffset, app->entities[i].localParamsSize);

			Submesh& submesh = mesh.submeshes[j];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}
	}

	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	programTexGeo = app->programs[app->screenRectProgramIdx];
	glUseProgram(programTexGeo.handle);
	glBindVertexArray(app->vao);

	// - set the blending state
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->globalUniformBuffer.handle, app->globalParamsOffset, app->globalParamsSize);

	// - bind the texture into unit 0
	glUniform1i(app->albedoTexture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.albedoTex);

	glUniform1i(app->normalsTexture, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.normalsTex);

	glUniform1i(app->worldPositionTexture, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.worldPosTex);

	glUniform1i(app->depthTexture, 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, app->framebuffer.depthTex);
	
	glUniform1i(app->drawModeUniform, (int)app->drawMode);

	// - draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// - clean
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